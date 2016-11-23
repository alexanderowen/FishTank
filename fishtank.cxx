/*
 * Author: Alexander Owen
 *
 * Renders a fish tank
 */

#include "vtkSmartPointer.h"
#include "vtkSphereSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkCallbackCommand.h"
#include "vtkCommand.h"
#include "vtkActor.h"
#include "vtkInteractorStyle.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkProperty.h"
#include "vtkCamera.h"
#include "vtkLight.h"
#include "vtkOpenGLPolyDataMapper.h"
#include "vtkJPEGReader.h"
#include "vtkImageData.h"

#include <vtkOBJReader.h>

#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkPolyDataReader.h>
#include <vtkPoints.h>
#include <vtkUnsignedCharArray.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkCellArray.h>

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
    
   void   IncrementSize()
   {
       size += 0.01;
       if (size > 2.0)
           size = 1.0;
   }

   void
   RemoveVTKOpenGLStateSideEffects()
   {
     float Info[4] = { 0, 0, 0, 1 };
     glLightModelfv(GL_LIGHT_MODEL_AMBIENT, Info);
     float ambient[4] = { 1,1, 1, 1.0 };
     glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
     float diffuse[4] = { 1, 1, 1, 1.0 };
     glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
     float specular[4] = { 1, 1, 1, 1.0 };
     glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
   }


   void SetupLight(void)
   {
       glEnable(GL_LIGHTING);
       glEnable(GL_LIGHT0);
       GLfloat diffuse0[4] = { 0.8, 0.8, 0.8, 1 };
       GLfloat ambient0[4] = { 0.2, 0.2, 0.2, 1 };
       GLfloat specular0[4] = { 0.0, 0.0, 0.0, 1 };
       GLfloat pos0[4] = { 1, 2, 3, 0 };
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
 public:
   static vtk441MapperPart1 *New();
   
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

    vtkSmartPointer<vtkPolyDataMapper> windowMapper =                                                       
      vtkSmartPointer<vtkPolyDataMapper>::New();                                                      
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

    // Sign up to receive TimerEvent
    vtkSmartPointer<vtkTimerCallback> cb = 
      vtkSmartPointer<vtkTimerCallback>::New();
    iren->AddObserver(vtkCommand::TimerEvent, cb);
    cb->SetMapper(windowMapper);
    cb->SetRenderWindow(windowRenderer);
    cb->SetCamera(renderer->GetActiveCamera());
   
    vtkSmartPointer<vtkCallbackCommand> keypressCallback = 
      vtkSmartPointer<vtkCallbackCommand>::New();
    keypressCallback->SetCallback ( KeypressCallbackFunction );
    iren->AddObserver ( vtkCommand::KeyPressEvent, keypressCallback );

    int timerId = iren->CreateRepeatingTimer(10);  // repeats every 10 microseconds <--> 0.01 seconds
    std::cout << "timerId: " << timerId << std::endl;  
   
    iren->Start();

    return EXIT_SUCCESS;
}

void KeypressCallbackFunction( vtkObject* caller, long unsigned int vtkNotUsed(eventId), void* vtkNotUsed(clientData), void* vtkNotUsed(callData) )
{
    std::cout << "Keypress callback" << std::endl;
   
    vtkRenderWindowInteractor *iren = 
      static_cast<vtkRenderWindowInteractor*>(caller);
   
    std::cout << "Pressed: " << iren->GetKeySym() << std::endl;
}




