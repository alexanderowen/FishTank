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

#include <string>



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

        static vtk441MapperPart1 *New();

        vtk441MapperPart1() 
        {
            rotateLeft   = false;
            rotateRight  = false;
            ascend       = false;
            descend      = false;
            moveForward  = false;
            moveBackward = false;
        }

    // RenderPiece is called whenever geometry to be rendered. If not overwritten, defaults to
    // superclass implementation

        virtual void RenderPiece(vtkRenderer *ren, vtkActor *act)
        {
            if (rotateLeft)
            {
                act->RotateY(1);
                rotateLeft = false;
            }
            if (rotateRight)
            {
                act->RotateY(-1);
                rotateRight = false;
            }
            if (ascend)
            {
                act->AddPosition(0, 1, 0);
                ascend = false;
            }
            if (descend)
            {
                act->AddPosition(0, -1, 0);
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
 
    virtual void Execute(vtkObject *vtkNotUsed(caller), unsigned long eventId,
                         void *vtkNotUsed(callData))
    {
      // THIS IS WHAT GETS CALLED EVERY TIMER
      //cout << "Got a timer!!" << this->TimerCount << endl;

      // NOW DO WHAT EVER ACTIONS YOU WANT TO DO...
      if (vtkCommand::TimerEvent == eventId)
        {
        ++this->TimerCount;
        }

      // Make a call to the mapper to make it alter how it renders...
      // AO: Doesn't seem necessary to me
      //if (mapper != NULL)
        //    mapper->IncrementSize();

      // Modify the camera...
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
    // Dummy input so VTK pipeline is happy.
    //
    vtkSmartPointer<vtkSphereSource> sphere =
      vtkSmartPointer<vtkSphereSource>::New();
    sphere->SetThetaResolution(100);
    sphere->SetPhiResolution(50);

    // The mapper is responsible for pushing the geometry into the graphics
    // library. It may also do color mapping, if scalars or other attributes
    // are defined. 
    //
  
    vtkSmartPointer<vtkOBJReader> reader =                                                            
    vtkSmartPointer<vtkOBJReader>::New();                                                           
    reader->SetFileName("../Models/obj/plane.obj");                                                            
    reader->Update(); 

    vtkSmartPointer<vtk441MapperPart1> windowMapper =                                                       
      vtkSmartPointer<vtk441MapperPart1>::New();                                                      
    windowMapper->SetInputConnection(reader->GetOutputPort()); 
/*
  vtkSmartPointer<vtk441MapperPart1> windowMapper =
    vtkSmartPointer<vtk441MapperPart1>::New();
  windowMapper->SetInputConnection(sphere->GetOutputPort());
*/
    vtkSmartPointer<vtkActor> windowActor =
      vtkSmartPointer<vtkActor>::New();
    windowActor->SetMapper(windowMapper);

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
    //
    renderer->AddActor(windowActor);
    renderer->SetBackground(0, 0, 0);
    windowRenderer->SetSize(600, 600);

    // Set up the lighting.
    //
    renderer->GetActiveCamera()->SetFocalPoint(0,0,0);
    renderer->GetActiveCamera()->SetPosition(0,0,70);
    renderer->GetActiveCamera()->SetViewUp(0,1,0);
    renderer->GetActiveCamera()->SetClippingRange(20, 120);
    renderer->GetActiveCamera()->SetDistance(170);
    
    // This starts the event loop and invokes an initial render.
    //
    ((vtkInteractorStyle *)iren->GetInteractorStyle())->SetAutoAdjustCameraClippingRange(0);
    iren->Initialize();

/*
    // Sign up to receive TimerEvent
    vtkSmartPointer<vtkTimerCallback> cb = 
      vtkSmartPointer<vtkTimerCallback>::New();
    iren->AddObserver(vtkCommand::TimerEvent, cb);
    cb->SetMapper(windowMapper);
    cb->SetRenderWindow(windowRenderer);
    cb->SetCamera(renderer->GetActiveCamera());
*/
    vtkSmartPointer<vtkCallbackCommand> keypressCallback = 
      vtkSmartPointer<vtkCallbackCommand>::New();
    keypressCallback->SetCallback ( KeypressCallbackFunction );
    iren->AddObserver ( vtkCommand::KeyPressEvent, keypressCallback );

/*
    int timerId = iren->CreateRepeatingTimer(10);  // repeats every 10 microseconds <--> 0.01 seconds
    std::cout << "timerId: " << timerId << std::endl;  
*/
    fish = windowMapper; 
    window = windowRenderer;

    iren->Start();

    return EXIT_SUCCESS;
}

void KeypressCallbackFunction( vtkObject* caller, long unsigned int vtkNotUsed(eventId), void* vtkNotUsed(clientData), void* vtkNotUsed(callData) )
{
    std::cout << "Keypress callback" << std::endl;
   
    vtkRenderWindowInteractor *iren = static_cast<vtkRenderWindowInteractor*>(caller);
    
    std::string key = iren->GetKeySym();
    //std::cout << "Pressed: " << iren->GetKeySym() << std::endl;
    if (key == "Left")
        fish->rotateLeft = true;
    else if (key == "Right")
        fish->rotateRight = true;
    else if (key == "a")
        fish->ascend = true;
    else if (key == "z")
        fish->descend = true;
    else if (key == "Up")
        fish->moveForward = true;
    else if (key == "Down")
        fish->moveBackward = true;
    window->Render();

}




