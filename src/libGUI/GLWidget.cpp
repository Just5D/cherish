#include "GLWidget.h"

#include <iostream>
#include <stdexcept>
#include <vector>

#include <QKeyEvent>
#include <QWheelEvent>

#include <osg/StateSet>
#include <osg/Material>
#include <osg/Camera>
#include <osgViewer/Viewer>
#include <osgGA/EventQueue>
#include <osgGA/EventQueue>
#include <osgGA/TrackballManipulator>

#include "SceneState.h"
#include "ViewerCommand.h"
#include "Manipulator.h"
#include "EventHandler.h"

GLWidget::GLWidget(RootScene *root, QUndoStack *stack, QWidget *parent, Qt::WindowFlags f)
    : QOpenGLWidget(parent, f)

    , m_GraphicsWindow(new osgViewer::GraphicsWindowEmbedded(this->x(), this->y(), this->width(), this->height()))
    , m_Viewer(new osgViewer::CompositeViewer)
    , m_RootScene(root)
    , m_TabletDevice(QTabletEvent::Stylus) // http://doc.qt.io/qt-5/qtabletevent.html#TabletDevice-enum

    , m_ModeView(1)
    , m_DeviceDown(false)
    , m_DeviceActive(false)
    , m_mouseMode(cher::PEN_SKETCH)

    , m_manipulator(new Manipulator(m_mouseMode))
    , m_EH(new EventHandler(this, m_RootScene.get(), m_mouseMode))

    , m_viewStack(stack)
{
    /* camera settings */
    float ratio = static_cast<float>(this->width()) / static_cast<float>( this->height());
    osg::Vec3 look = osg::Vec3(osg::Y_AXIS);
    osg::Vec3 up = osg::Vec3(osg::Z_AXIS);
    osg::Vec3 center = m_RootScene->getBound().center();
    double radius = m_RootScene->getBound().radius();
    osg::Camera* camera = new osg::Camera;
    camera->setViewport(0,0,this->width(), this->height());
    camera->setProjectionMatrixAsPerspective(30.f, ratio, 1.f, 1000.f);
    camera->setViewMatrixAsLookAt(center-look*(radius*3.f), center, up);
    camera->setGraphicsContext(m_GraphicsWindow.get());
    camera->setClearColor(cher::BACKGROUND_CLR);
    camera->setName("Camera");

    /* view settings */
    osg::ref_ptr< osgViewer::View> view = new osgViewer::View;
    view->setCamera(camera);
    view->setSceneData(m_RootScene.get());
    view->setCameraManipulator(m_manipulator.get());
    view->addEventHandler(m_EH.get());

    /* manipulator settings */
    m_manipulator->setAllowThrow(false);
    m_manipulator->getTransformation(m_eye, m_center, m_up);

    // viewer settings
    m_Viewer->addView(view.get());
    m_Viewer->setThreadingModel(osgViewer::CompositeViewer::SingleThreaded);
    m_Viewer->realize();

    // widget settings
    this->setFocusPolicy(Qt::StrongFocus);
    this->setMouseTracking(true);
}

GLWidget::~GLWidget()
{
}

osg::Camera *GLWidget::getCamera() const
{
    if (!m_Viewer.get()){
        std::cerr << "GLWindget getCamera: could not obtain viewer" << std::endl;
        return NULL;
    }

    if (!m_Viewer->getView(0))
    {
        std::cerr << "GLWindget getCamera: could not obtain view" << std::endl;
        return NULL;
    }
    return m_Viewer->getView(0)->getCamera();
    /*std::vector<osg::Camera*> cameras;
    this->m_Viewer->getCameras(cameras);
    if (cameras.empty())
        return NULL;
    else
        return cameras.at(0);*/
}

void GLWidget::setCameraView()
{
    osg::Vec3d eye, center, up;
    m_manipulator->getTransformation(eye, center, up);
    if (eye == m_eye && center == m_center && up == m_up) return;

    osg::Vec3d de = eye - m_eye;
    osg::Vec3d dc = center - m_center;
    osg::Vec3d du = up - m_up;

    m_manipulator->setTransformation(m_eye, m_center, m_up);

    ViewerCommand* cmd = new ViewerCommand(this, m_manipulator.get(),
                                           m_eye, m_center, m_up,
                                           de, dc, du);
    m_viewStack->push(cmd);
}

void GLWidget::setCameraView(const osg::Vec3d &eye, const osg::Vec3d &center, const osg::Vec3d &up, const double &fov)
{
    m_manipulator->setTransformation(eye, center, up);
    osg::Camera* camera = this->getCamera();
    if (camera){
        float ratio = static_cast<float>(this->width()) / static_cast<float>( this->height());
        camera->setProjectionMatrixAsPerspective(fov*0.5, ratio, 1.f, 1000.f);
        emit this->FOVSet(fov);
    }

    /* for stack of camera views */
    this->setCameraView();
}

void GLWidget::getCameraView(osg::Vec3d &eye, osg::Vec3d &center, osg::Vec3d &up, double &fov) const
{
    m_manipulator->getTransformation(eye, center, up);
    osg::Camera* camera = this->getCamera();
    if (!camera) fov = 30.f;
    else {
        double ar, zn, zf;
        camera->getProjectionMatrixAsPerspective(fov, ar, zn, zf);
    }
    fov *= 2.; // OSG accepts half angle, but the widget takes the whole angle value
}

void GLWidget::setMouseMode(const cher::MOUSE_MODE &mode)
{
    m_mouseMode = mode;
    m_manipulator->setMode(m_mouseMode);
    m_EH->setMode(m_mouseMode);
    emit this->mouseModeSet(m_mouseMode);
}

void GLWidget::setTabletActivity(bool active)
{
    m_DeviceActive = active;
}

void GLWidget::onRequestScreenshot(QPixmap &pmap, const osg::Vec3d &eye, const osg::Vec3d &center, const osg::Vec3d &up)
{
    /* save the current camera view */
    osg::Vec3d eye_, center_, up_;
    double fov_;
    this->getCameraView(eye_, center_, up_, fov_);

    /* move the camera to bookmark view */
    m_manipulator->setTransformation(eye, center, up);


    /* save the current scene state */
    osg::ref_ptr<entity::SceneState> ss = new entity::SceneState();
    ss->stripDataFrom(m_RootScene.get());

    /* make sure tools are off */
    m_RootScene->setToolsVisibility(false);
    this->update();

    /* grab the screenshot */
    pmap = this->grab();

    /* apply the saved scene state */
    m_RootScene->setSceneState(ss);

    /* return to the current camera view */
    m_manipulator->setTransformation(eye_, center_, up_);
    this->update();
}

/* FOV is a whole angle, not half angle */
void GLWidget::onFOVChangedSlider(double fov)
{
    osg::Camera* camera = this->getCamera();
    if (!camera){
        outErrMsg("onFOVChangedSlider: could not obtain camera ptr");
        return;
    }
    float ratio = static_cast<float>(this->width()) / static_cast<float>( this->height());
    camera->setProjectionMatrixAsPerspective(fov*0.5, ratio, 1.f, 1000.f);
    camera->dirtyBound();
    this->update();
}

void GLWidget::onOrthoSet(bool ortho)
{
    osg::Camera* camera = this->getCamera();
    if (!camera){
        outErrMsg("onFOVChanged: could not obtain camera ptr");
        return;
    }
    float ratio = static_cast<float>(this->width()) / static_cast<float>( this->height());
    if (!ortho)
        camera->setProjectionMatrixAsPerspective(60, ratio, 1.f, 1000.f);
    else{
        outLogMsg("setting camera to ortho");
        camera->setProjectionMatrixAsOrtho(-1,1,-1,1,-1,1);
    }
    camera->dirtyBound();
    this->update();
}

void GLWidget::initializeGL()
{
    osg::StateSet* stateSet = m_RootScene->getOrCreateStateSet();
    osg::Material* material = new osg::Material;
    material->setColorMode(osg::Material::AMBIENT_AND_DIFFUSE);
    stateSet->setAttributeAndModes(material, osg::StateAttribute::ON);
    stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
}

void GLWidget::paintGL()
{
    this->m_Viewer->frame();
}

void GLWidget::resizeGL(int w, int h)
{
    this->getEventQueue()->windowResize(this->x(), this->y(), w, h);
    this->m_GraphicsWindow->resized(this->x(), this->y(), w, h);
    this->onResize(w, h);
}

void GLWidget::keyPressEvent(QKeyEvent *event)
{
    QString keystr = event->text();
    const char* keydat = keystr.toLocal8Bit().data();
    switch (event->key()) {
    case Qt::Key_H:
        this->onHome();
        return;
    case Qt::Key_A:
        if ((event->modifiers() & Qt::ControlModifier))
            m_RootScene->selectAllEntities();
        return;
    default:
        break;
    }
    if (event->modifiers() & Qt::ControlModifier){
        if (cher::maskMouse & cher::MOUSE_SELECT)
            this->setMouseMode(cher::SELECT_CANVAS);
        std::cout << "Qt ctrl ON" << std::endl;
        this->getEventQueue()->keyPress(osgGA::GUIEventAdapter::KEY_Control_L);
    }

    this->getEventQueue()->keyPress(osgGA::GUIEventAdapter::KeySymbol (*keydat));
}

void GLWidget::keyReleaseEvent(QKeyEvent *event)
{
    /* http://stackoverflow.com/questions/20746488/how-to-catch-ctrl-key-release */
    if (event->key() == Qt::Key_Control){
        if (cher::maskMouse & cher::MOUSE_SELECT)
            this->setMouseMode(cher::SELECT_ENTITY);
        std::cout << "Qt ctrl OFF" << std::endl;
        this->getEventQueue()->keyRelease(osgGA::GUIEventAdapter::KEY_Control_L);
    }

    QString keystr = event->text();
    const char* keydat = keystr.toLocal8Bit().data();
    this->getEventQueue()->keyRelease(osgGA::GUIEventAdapter::KeySymbol(*keydat));
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    /* to detect pen's hovering events we only check pen is not touching the ground,
     * but we have to pass the mouse move when pen is hovering, i.e. device is active */
    if (!m_DeviceDown) {
        this->getEventQueue()->mouseMotion(static_cast<float>(event->x()), static_cast<float>(event->y()));
    }
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    if (!m_DeviceDown && !m_DeviceActive) {
        unsigned int button = 0;
        switch( event->button() ) {
        case Qt::LeftButton:
            button = 1;
            break;
        case Qt::MiddleButton:
            button = 2;
            break;
        case Qt::RightButton:
            button = 3;
            break;
        default:
            break;
        }
        this->getEventQueue()->mouseButtonPress(static_cast<float>(event->x()), static_cast<float>(event->y()), button);
    }
}

void GLWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    outLogMsg("double click detected");
    emit this->autoSwitchMode(m_mouseMode);
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_DeviceDown && !m_DeviceActive) {
        unsigned int button = 0;
        switch( event->button() ) {
        case Qt::LeftButton:
            button = 1;
            break;
        case Qt::MiddleButton:
            button = 2;
            break;
        case Qt::RightButton:
            button = 3;
            break;
        default:
            break;
        }
        this->getEventQueue()->mouseButtonRelease(static_cast<float>(event->x()), static_cast<float>(event->y()), button);
    }

    this->setCameraView();
    m_manipulator->getTransformation(m_eye, m_center, m_up);
}

void GLWidget::wheelEvent(QWheelEvent *event)
{
    event->accept();
    int delta = event->delta();
    osgGA::GUIEventAdapter::ScrollingMotion motion = delta > 0 ?   osgGA::GUIEventAdapter::SCROLL_UP
                                                                 : osgGA::GUIEventAdapter::SCROLL_DOWN;
    this->getEventQueue()->mouseScroll( motion );

    this->setCameraView();
    m_manipulator->getTransformation(m_eye, m_center, m_up);
}

void GLWidget::tabletEvent(QTabletEvent *event)
{
    event->accept();
    this->getEventQueue()->penPressure(static_cast<float>(event->pressure()));
    this->getEventQueue()->penOrientation(static_cast<float>(event->xTilt()), static_cast<float>(event->yTilt()),
                                          static_cast<float>(event->rotation()));

    switch (event->pointerType()){
    case QTabletEvent::Pen:
        this->getEventQueue()->penProximity(osgGA::GUIEventAdapter::PEN, m_DeviceActive);
        break;
    case QTabletEvent::Eraser:
        this->getEventQueue()->penProximity(osgGA::GUIEventAdapter::ERASER, m_DeviceActive);
        break;
    case QTabletEvent::Cursor:
        this->getEventQueue()->penProximity(osgGA::GUIEventAdapter::PUCK, m_DeviceActive);
        break;
    default:
        this->getEventQueue()->penProximity(osgGA::GUIEventAdapter::UNKNOWN, m_DeviceActive);
        break;
    }

    // for more on buttons(), see http://doc.qt.io/qt-5/qtabletevent.html#buttons
    switch (event->type()){
    case QEvent::TabletPress:
        // do not process PRESS if the device was already pressed
        // it means we will ignore any buttons pressed on stylus
        if (m_DeviceDown)
            return;
        m_DeviceDown = true;
        this->getEventQueue()->mouseButtonPress(static_cast<float>(event->x()), static_cast<float>(event->y()), 1);
        break;
    case QEvent::TabletRelease:
        // do not process RELEASE if the device was already released
        // it means we will ignore any buttons pressed on stylus
        if (!m_DeviceDown)
            return;
        m_DeviceDown = false;
        this->getEventQueue()->mouseButtonRelease(static_cast<float>(event->x()), static_cast<float>(event->y()), 1);

        this->setCameraView();
        m_manipulator->getTransformation(m_eye, m_center, m_up);
        break;
    case QEvent::TabletMove:
        this->getEventQueue()->mouseMotion(static_cast<float>(event->x()), static_cast<float>(event->y()));
        break;
    default:
        break;
    }
}

bool GLWidget::event(QEvent *event)
{
    bool handled = QOpenGLWidget::event(event);
    switch(event->type()){
   case QEvent::TabletMove:
    case QEvent::TabletPress:
    case QEvent::TabletRelease:
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove:
    case QEvent::Wheel:
        this->update();
        break;
    default:
        break;
    }
    return handled;
}

void GLWidget::onHome()
{
    osgViewer::ViewerBase::Views views;
    m_Viewer->getViews(views);
    for (unsigned int i=0; i<views.size(); ++i){
        osgViewer::View* view = views.at(i);
        view->home();

        this->setCameraView();
        m_manipulator->getTransformation(m_eye, m_center, m_up);
    }
}

void GLWidget::onResize(int w, int h)
{
    std::vector<osg::Camera*> cameras;
    this->m_Viewer->getCameras(cameras);
    if (cameras.size() != static_cast<unsigned int>(this->m_ModeView)) {
        outErrMsg("onResize(): the cameras number does not correspond to view mode");
        outLogVal("Camera size", cameras.size());
        outLogVal("View mode", this->m_ModeView);
        return;
    }
    if (this->m_ModeView == 1)
        cameras[0]->setViewport( 0,0,this->width(), this->height() );
    else if (this->m_ModeView == 2){
        cameras[0]->setViewport( 0,0,this->width()/2, this->height() );
        cameras[1]->setViewport( this->width() / 2, 0, this->width() / 2, this->height() );
    }
    else if (this->m_ModeView == 3){
        cameras[0]->setViewport( 0,0,this->width()/3, this->height() );
        cameras[1]->setViewport( this->width() / 3, 0, this->width()/ 3, this->height() );
        cameras[2]->setViewport( this->width() * 2 / 3, 0, this->width()/ 3, this->height() );
    }
    else{
        outErrMsg("onResize(): unsupported view mode");
        outLogVal("Cameras number", cameras.size());
        return;
    }
}

osgGA::EventQueue *GLWidget::getEventQueue() const
{
    osgGA::EventQueue* eventQueue = m_GraphicsWindow->getEventQueue();
    if( eventQueue )
        return eventQueue;
    else
        throw std::runtime_error( "Unable to obtain valid event queue");
}

