/*
 * Author: Alexander Owen
 *
 * Renders a fish tank
 */

#include <vtkActor.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCommand.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkInteractorStyle.h>
#include <vtkJPEGReader.h>
#include <vtkLight.h>
#include <vtkOBJReader.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataReader.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkUnsignedCharArray.h>

#include <vtkInteractorStyleSwitch.h>
#include <vtkLight.h>
#include <vtkInteractorStyleJoystickActor.h>
#include <vtkInteractorStyleJoystickCamera.h>
#include <vtkProperty.h>
#include <vtkIndent.h>
#include <vtkLightCollection.h>

#include <string>
#include <sys/timeb.h>
#include <sys/types.h>


class vtk441Mapper : public vtkOpenGLPolyDataMapper
{
    protected:
        GLuint displayList;
        bool   initialized;
        float  size;

    public:
        vtk441Mapper()
        {
          initialized = false;
          size = 1;
        }
         
        void IncrementSize()
        {
            size += 0.01;
            if (size > 2.0)
                size = 1.0;
        }

        void
        RemoveVTKOpenGLStateSideEffects()
        {
          float Info[4]     = { 0, 0, 0, 1 };
          float ambient[4]  = { 1,1, 1, 1.0 };
          float diffuse[4]  = { 1, 1, 1, 1.0 };
          float specular[4] = { 1, 1, 1, 1.0 };
          glLightModelfv(GL_LIGHT_MODEL_AMBIENT, Info);
          glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
          glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
          glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
        }

        void SetupLight(void)
        {
            glEnable(GL_LIGHTING);
            glEnable(GL_LIGHT0);
            GLfloat diffuse0[4]  = { 0.8, 0.8, 0.8, 1 };
            GLfloat ambient0[4]  = { 0.2, 0.2, 0.2, 1 };
            GLfloat specular0[4] = { 0.0, 0.0, 0.0, 1 };
            GLfloat pos0[4]      = { 1, 2, 3, 0 };
            glLightfv(GL_LIGHT0, GL_POSITION, pos0);
            glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0);
            glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0);
            glLightfv(GL_LIGHT0, GL_SPECULAR, specular0);
            glDisable(GL_LIGHT1);
            glDisable(GL_LIGHT2);
            glDisable(GL_LIGHT3);
            glDisable(GL_LIGHT5);
            glDisable(GL_LIGHT6);
            glDisable(GL_LIGHT7);
        }
};

int getMilliCount(){
        timeb tb;
        ftime(&tb);
        int nCount = tb.millitm + (tb.time & 0xfffff) * 1000;
        return nCount;
}

int getMilliSpan(int nTimeStart){
        int nSpan = getMilliCount() - nTimeStart;
        if(nSpan < 0)
            nSpan += 0x100000 * 1000;
        return nSpan;
}

class vtk441MapperPart1 : public vtk441Mapper
{
    private:
        typedef vtk441Mapper super;

    public:
        bool rotateLeft;
        bool rotateRight; 
        bool ascend;
        bool descend;
        bool moveForward;
        bool moveBackward;
        int lastTime;

        static vtk441MapperPart1 *New();

        vtk441MapperPart1() 
        {
            rotateLeft   = false;
            rotateRight  = false;
            ascend       = false;
            descend      = false;
            moveForward  = false;
            moveBackward = false;
            lastTime     = 0; 
        }

        // RenderPiece is called whenever geometry to be rendered. If not overwritten, defaults to
        // superclass implementation
        virtual void RenderPiece(vtkRenderer *ren, vtkActor *act)
        {
            int curTime = getMilliCount(); 
            double delta = getMilliSpan(lastTime) * 0.001;
            double degrees = 45 * delta;
            double position = 10 * delta;
            //cerr << delta << endl;
            lastTime = curTime;
            if (rotateLeft)
            {
                act->RotateY(degrees);
                rotateLeft = false;
            }
            if (rotateRight)
            {
                act->RotateY(-degrees);
                rotateRight = false;
            }
            if (ascend)
            {
                act->AddPosition(0, position, 0);
                ascend = false;
            }
            if (descend)
            {
                act->AddPosition(0, -position, 0);
                descend = false;
            }
            if (moveForward)
            {
                act->AddPosition(1, 0, 0);
                moveForward = false;
            }
            if (moveBackward)
            {
                act->AddPosition(-1, 0, 0);
                moveBackward = false;
            }
            super::RenderPiece(ren, act);    

            /* Code to draw axes*/
            glEnable(GL_COLOR_MATERIAL);
            glBegin(GL_LINES);
            glColor3ub(255, 0, 0);
            glVertex3f(0, 0, 0);
            glVertex3f(10, 0, 0);
            glColor3ub(0, 255, 0);
            glVertex3f(0, 0, 0);
            glVertex3f(0, 10, 0);
            glColor3ub(0, 0, 255);
            glVertex3f(0, 0, 0);
            glVertex3f(0, 0, 10);
            glEnd();
        }
        
   /*     
        virtual void RenderPiece(vtkRenderer *ren, vtkActor *act)
        {
           RemoveVTKOpenGLStateSideEffects();
           SetupLight();
           glBegin(GL_TRIANGLES);
           glVertex3f(-10*size, -10*size, -10*size);
           glVertex3f(10*size, -10*size, 10*size);
           glVertex3f(10*size, 10*size, 10*size);
           glEnd();
        }
    */
};

vtkStandardNewMacro(vtk441MapperPart1);

class vtkTimerCallback : public vtkCommand
{
    public:
        static vtkTimerCallback *New()
        {
            vtkTimerCallback *cb = new vtkTimerCallback;
            cb->TimerCount = 0;
            cb->mapper = NULL;
            cb->renWin = NULL;
            cb->cam    = NULL;
            cb->angle  = 0;
            return cb;
        }

        void   SetMapper(vtkPolyDataMapper *m) { mapper = m; };
        void   SetRenderWindow(vtkRenderWindow *rw) { renWin = rw; };
        void   SetCamera(vtkCamera *c) { cam = c; };
 
        virtual void Execute(vtkObject *vtkNotUsed(caller), unsigned long eventId, void *vtkNotUsed(callData))
        {
            // THIS IS WHAT GETS CALLED EVERY TIMER
            //cout << "Got a timer!!" << this->TimerCount << endl;
      
            // NOW DO WHAT EVER ACTIONS YOU WANT TO DO...
            /*
            if (vtkCommand::TimerEvent == eventId)
            {
                ++this->TimerCount;
            }
            */
      
            // Make a call to the mapper to make it alter how it renders...
            // AO: Doesn't seem necessary to me
            //if (mapper != NULL)
              //    mapper->IncrementSize();
      
            // Modify the camera...
            /*
            if (cam != NULL)
            {
               cam->SetFocalPoint(0,0,0);
               float rads = angle/360.0*2*3.14;
               cam->SetPosition(70*cos(rads),0,70*sin(rads));
               angle++;
               if (angle > 360)
                  angle = 0;
               cam->SetViewUp(0,1,0);
               cam->SetClippingRange(20, 120);
               cam->SetDistance(70);
            }
            */
      
            // Force a render...
            if (renWin != NULL)
                renWin->Render();
        }   
 
    private:
        int TimerCount;
        vtkPolyDataMapper *mapper;
        vtkRenderWindow *renWin;
        vtkCamera *cam;
        float angle;
};


void KeypressCallbackFunction (vtkObject* caller, long unsigned int eventId, void* clientData, void* callData);

/* GLOBALS */
vtk441MapperPart1 *fish;                                                      
vtkRenderWindow   *window;

int main()
{
    // The mapper is responsible for pushing the geometry into the graphics
    // library. It may also do color mapping, if scalars or other attributes
    // are defined. 
 
    // PURPLE FISH 
    vtkSmartPointer<vtkOBJReader> reader = vtkSmartPointer<vtkOBJReader>::New();                                                           
    reader->SetFileName("../Models/obj/fish1.obj");                                                            
    reader->Update(); 

    vtkSmartPointer<vtk441MapperPart1> windowMapper = vtkSmartPointer<vtk441MapperPart1>::New();                                                      
    windowMapper->SetInputConnection(reader->GetOutputPort()); 

    vtkSmartPointer<vtkActor> windowActor = vtkSmartPointer<vtkActor>::New();

    vtkSmartPointer<vtkProperty> prop1 = vtkSmartPointer<vtkProperty>::New();
    prop1->SetDiffuseColor(.8, .126, .756);
    windowActor->SetMapper(windowMapper);
    windowActor->SetProperty(prop1);
    windowActor->SetPosition(-11, 0, 0);

    // BLUE FISH 
    vtkSmartPointer<vtkOBJReader> reader3 = vtkSmartPointer<vtkOBJReader>::New();                                                           
    reader3->SetFileName("../Models/obj/fish2.obj");                                                            
    reader3->Update(); 

    vtkSmartPointer<vtk441MapperPart1> windowMapper3 = vtkSmartPointer<vtk441MapperPart1>::New();                                                      
    windowMapper3->SetInputConnection(reader3->GetOutputPort()); 

    vtkSmartPointer<vtkActor> windowActor3 = vtkSmartPointer<vtkActor>::New();

    vtkSmartPointer<vtkProperty> prop3 = vtkSmartPointer<vtkProperty>::New();
    prop3->SetDiffuseColor(.011, .103, 1.0);
    windowActor3->SetMapper(windowMapper3);
    windowActor3->SetProperty(prop3);
    windowActor3->SetPosition(-9, 0, 0);

    // CORAL 
    vtkSmartPointer<vtkOBJReader> reader4 = vtkSmartPointer<vtkOBJReader>::New();                                                           
    reader4->SetFileName("../Models/obj/coral1.obj");
    reader4->Update(); 

    vtkSmartPointer<vtk441MapperPart1> windowMapper4 = vtkSmartPointer<vtk441MapperPart1>::New();                                                      
    windowMapper4->SetInputConnection(reader4->GetOutputPort()); 

    vtkSmartPointer<vtkActor> windowActor4 = vtkSmartPointer<vtkActor>::New();

    vtkSmartPointer<vtkProperty> prop4 = vtkSmartPointer<vtkProperty>::New();
    prop4->SetDiffuseColor(1.0, 0.065, 0.865);
    windowActor4->SetMapper(windowMapper4);
    windowActor4->SetProperty(prop4);
    windowActor4->SetPosition(-9, -10, 0);

    // YELLOW FISH 
    vtkSmartPointer<vtkOBJReader> reader5 = vtkSmartPointer<vtkOBJReader>::New();                                                           
    reader5->SetFileName("../Models/obj/fish3.obj");
    reader5->Update(); 

    vtkSmartPointer<vtk441MapperPart1> windowMapper5 = vtkSmartPointer<vtk441MapperPart1>::New();                                                      
    windowMapper5->SetInputConnection(reader5->GetOutputPort()); 

    vtkSmartPointer<vtkActor> windowActor5 = vtkSmartPointer<vtkActor>::New();

    vtkSmartPointer<vtkProperty> prop5 = vtkSmartPointer<vtkProperty>::New();
    prop5->SetDiffuseColor(1.0, 0.854, 0.0);
    windowActor5->SetMapper(windowMapper5);
    windowActor5->SetProperty(prop5);
    windowActor5->SetPosition(-9, 10, 0);

    // Leaf 1
    vtkSmartPointer<vtkOBJReader> reader6 = vtkSmartPointer<vtkOBJReader>::New();                                                           
    reader6->SetFileName("../Models/obj/leaf1.obj");
    reader6->Update(); 

    vtkSmartPointer<vtk441MapperPart1> windowMapper6 = vtkSmartPointer<vtk441MapperPart1>::New();                                                      
    windowMapper6->SetInputConnection(reader6->GetOutputPort()); 

    vtkSmartPointer<vtkActor> windowActor6 = vtkSmartPointer<vtkActor>::New();

    vtkSmartPointer<vtkProperty> prop6 = vtkSmartPointer<vtkProperty>::New();
    prop6->SetDiffuseColor(0.0, 0.8, 0.0);
    windowActor6->SetMapper(windowMapper6);
    windowActor6->SetProperty(prop6);
    windowActor6->SetPosition(9, 0, 0);

    // Submarine 
    vtkSmartPointer<vtkOBJReader> reader7 = vtkSmartPointer<vtkOBJReader>::New();                                                           
    reader7->SetFileName("../Models/obj/submarine.obj");
    reader7->Update(); 

    vtkSmartPointer<vtk441MapperPart1> windowMapper7 = vtkSmartPointer<vtk441MapperPart1>::New();                                                      
    windowMapper7->SetInputConnection(reader7->GetOutputPort()); 

    vtkSmartPointer<vtkActor> windowActor7 = vtkSmartPointer<vtkActor>::New();

    vtkSmartPointer<vtkProperty> prop7 = vtkSmartPointer<vtkProperty>::New();
    prop7->SetDiffuseColor(0.012, 0.342, 0.01);
    windowActor7->SetMapper(windowMapper7);
    windowActor7->SetProperty(prop7);
    windowActor7->SetPosition(13, -5, 0);

    // Tree
    vtkSmartPointer<vtkOBJReader> reader8 = vtkSmartPointer<vtkOBJReader>::New();                                                           
    reader8->SetFileName("../Models/obj/tree1.obj");
    reader8->Update(); 

    vtkSmartPointer<vtk441MapperPart1> winMap8 = vtkSmartPointer<vtk441MapperPart1>::New();                                                      
    winMap8->SetInputConnection(reader8->GetOutputPort()); 

    vtkSmartPointer<vtkActor> winAct8 = vtkSmartPointer<vtkActor>::New();

    vtkSmartPointer<vtkProperty> prop8 = vtkSmartPointer<vtkProperty>::New();
    prop8->SetDiffuseColor(0.016, 0.8, 0.035);
    winAct8->SetMapper(winMap8);
    winAct8->SetProperty(prop8);
    winAct8->SetPosition(8, -5, 0);

    // Tree spire 
    vtkSmartPointer<vtkOBJReader> r9 = vtkSmartPointer<vtkOBJReader>::New();                                                           
    r9->SetFileName("../Models/obj/treespire.obj");
    r9->Update(); 

    vtkSmartPointer<vtk441MapperPart1> winMap9 = vtkSmartPointer<vtk441MapperPart1>::New();                                                      
    winMap9->SetInputConnection(r9->GetOutputPort()); 

    vtkSmartPointer<vtkActor> winAct9 = vtkSmartPointer<vtkActor>::New();

    vtkSmartPointer<vtkProperty> prop9 = vtkSmartPointer<vtkProperty>::New();
    prop9->SetDiffuseColor(1.0, 0.089, 0.129);
    winAct9->SetMapper(winMap9);
    winAct9->SetProperty(prop9);
    winAct9->SetPosition(0, 0, -5);

    //FLOOR
    vtkSmartPointer<vtkOBJReader> reader2 =                                                            
      vtkSmartPointer<vtkOBJReader>::New();                                                           
    reader2->SetFileName("../Models/obj/floor.obj");
    reader2->Update(); 

    vtkSmartPointer<vtk441MapperPart1> windowMapper2 =                                                       
      vtkSmartPointer<vtk441MapperPart1>::New();                                                      
    windowMapper2->SetInputConnection(reader2->GetOutputPort()); 

    vtkSmartPointer<vtkActor> windowActor2 =
      vtkSmartPointer<vtkActor>::New();
    vtkSmartPointer<vtkProperty> p = vtkSmartPointer<vtkProperty>::New();
    p->SetDiffuseColor(0.0,0.0,0.45);
    windowActor2->SetMapper(windowMapper2);
    windowActor2->SetProperty(p);
    windowActor2->SetScale(5.5);
    windowActor2->SetPosition(0, -10, 0); 

// END NEW STUFF
    vtkSmartPointer<vtkRenderer> renderer =
      vtkSmartPointer<vtkRenderer>::New();

    vtkSmartPointer<vtkRenderWindow> windowRenderer =
      vtkSmartPointer<vtkRenderWindow>::New();
    windowRenderer->AddRenderer(renderer);
    renderer->SetViewport(0, 0, 1, 1);

    vtkSmartPointer<vtkRenderWindowInteractor> iren =
      vtkSmartPointer<vtkRenderWindowInteractor>::New();
    iren->SetRenderWindow(windowRenderer);

    // Add the actors to the renderer, set the background and size.
    renderer->AddActor(windowActor);
    renderer->AddActor(windowActor2);
    renderer->AddActor(windowActor3);
    renderer->AddActor(windowActor4);
    renderer->AddActor(windowActor5);
    renderer->AddActor(windowActor6);
    renderer->AddActor(windowActor7);
    renderer->AddActor(winAct8);
    renderer->AddActor(winAct9);
    renderer->SetBackground(0, 0, 0);
    //renderer->SetBackground2(255, 255, 255);
    //renderer->GradientBackgroundOn();
    windowRenderer->SetSize(650, 650);

    // Set up the lighting.
    renderer->GetActiveCamera()->SetFocalPoint(0,0,0);
    renderer->GetActiveCamera()->SetPosition(0,0,70);
    renderer->GetActiveCamera()->SetViewUp(0,1,0);
    renderer->GetActiveCamera()->SetClippingRange(20, 120);
    renderer->GetActiveCamera()->SetDistance(170);

    //DEBUG
    //renderer->GetLights()->PrintSelf(cerr, vtkIndent());
    //renderer->GetLights()->Print(cerr);
    renderer->RemoveAllLights();
    vtkSmartPointer<vtkLight> l = vtkSmartPointer<vtkLight>::New();
    l->SetAmbientColor(0.15, 0.15, 0.15); //doesn't seem to do anything :^/
    l->SetPosition(0.5,0.5,0.5);
    l->SetFocalPoint(0,0,0);
    renderer->AddLight(l);
    
   
    vtkSmartPointer<vtkInteractorStyleJoystickCamera> style = 
      vtkSmartPointer<vtkInteractorStyleJoystickCamera>::New();
  
    iren->SetInteractorStyle( style ); 
    // This starts the event loop and invokes an initial render.
    //((vtkInteractorStyle *)iren->GetInteractorStyle())->SetAutoAdjustCameraClippingRange(0);
    iren->Initialize();
/*
    // Sign up to receive TimerEvent
    vtkSmartPointer<vtkTimerCallback> cb = 
      vtkSmartPointer<vtkTimerCallback>::New();
    iren->AddObserver(vtkCommand::TimerEvent, cb);
    cb->SetMapper(windowMapper);
    cb->SetRenderWindow(windowRenderer);
    cb->SetCamera(renderer->GetActiveCamera());

    // Setup keypress event handling
    vtkSmartPointer<vtkCallbackCommand> keypressCallback = 
      vtkSmartPointer<vtkCallbackCommand>::New();
    keypressCallback->SetCallback ( KeypressCallbackFunction );
    iren->AddObserver ( vtkCommand::KeyPressEvent, keypressCallback );
*/
    //int timerId = iren->CreateRepeatingTimer(30);  // repeats every 30 milliseconds, ~30 FPS
    //std::cout << "timerId: " << timerId << std::endl;  

    fish = windowMapper; 
    window = windowRenderer;

    iren->Start();

    return EXIT_SUCCESS;
}

void KeypressCallbackFunction( vtkObject* caller, long unsigned int vtkNotUsed(eventId), void* vtkNotUsed(clientData), void* vtkNotUsed(callData) )
{
    //std::cout << "Keypress callback" << std::endl;
   
    vtkRenderWindowInteractor *iren = static_cast<vtkRenderWindowInteractor*>(caller);
    
    std::string key = iren->GetKeySym();
    //std::cout << "Pressed: " << iren->GetKeySym() << std::endl;
    if (key == "Left")
        fish->rotateLeft = true;
    if (key == "Right")
        fish->rotateRight = true;
    if (key == "a")
        fish->ascend = true;
    if (key == "z")
        fish->descend = true;
    if (key == "Up")
        fish->moveForward = true;
    if (key == "Down")
        fish->moveBackward = true;
    //window->Render();

}




