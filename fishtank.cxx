/*
 * Author: Alexander Owen
 *
 * Renders a fish tank in VTK with models in the .obj format
 * Uses VTK's OpenGL library
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

/**************
 *
 * Utility functions 
 *
 **************/

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

/*************
 *
 * VTK OpenGL classes
 *
 * ***********/

/* Class to extend VTK's OpenGL mapper */
class vtkCustomMapper : public vtkOpenGLPolyDataMapper
{
    protected:
        GLuint displayList;
        bool   initialized;
        float  size;

    public:
        vtkCustomMapper()
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

        /* Reset phong lighting coefficients */
        void RemoveVTKOpenGLStateSideEffects()
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

        /* Set the global lighting coefficients  */
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

/* Class to extend OpenGL mapper  */
class vtkCustomMapperP : public vtkCustomMapper
{
    private:
        typedef vtkCustomMapper super;

    public:
        bool rotateLeft;
        bool rotateRight; 
        bool ascend;
        bool descend;
        bool moveForward;
        bool moveBackward;
        int lastTime;

        bool displayAxes;

        static vtkCustomMapperP *New();

        vtkCustomMapperP() 
        {
            rotateLeft   = false;
            rotateRight  = false;
            ascend       = false;
            descend      = false;
            moveForward  = false;
            moveBackward = false;
            lastTime     = 0; 
            displayAxes  = false;
        }

        // RenderPiece is called whenever geometry to be rendered. If not overwritten, defaults to
        // superclass implementation
        virtual void RenderPiece(vtkRenderer *ren, vtkActor *act)
        {
            int curTime = getMilliCount(); 
            double delta = getMilliSpan(lastTime) * 0.001;
            double degrees = 45 * delta;
            double position = 10 * delta;
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
            if (displayAxes) 
            {
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
        }
};
vtkStandardNewMacro(vtkCustomMapperP);


/***************
 *
 * Main function
 *
 * *************/

/* Global variables */
vtkCustomMapperP  *fish;                                                      
vtkRenderWindow   *window;

int main()
{
    /** Gold Fish **/
    vtkSmartPointer<vtkOBJReader> GoldFishReader = vtkSmartPointer<vtkOBJReader>::New();                                                           
    GoldFishReader->SetFileName("../Models/obj/fish1.obj");                                                            
    GoldFishReader->Update(); 

    vtkSmartPointer<vtkCustomMapperP> goldFishMapper = vtkSmartPointer<vtkCustomMapperP>::New();                                                      
    goldFishMapper->SetInputConnection(GoldFishReader->GetOutputPort()); 

    vtkSmartPointer<vtkActor> goldFishActor = vtkSmartPointer<vtkActor>::New();
    vtkSmartPointer<vtkProperty> goldFishProp = vtkSmartPointer<vtkProperty>::New();
    goldFishProp->SetDiffuseColor(1, .426, .0);
    goldFishActor->SetMapper(goldFishMapper);
    goldFishActor->SetProperty(goldFishProp);
    goldFishActor->SetPosition(17, -5, 0);
    goldFishActor->RotateY(90);

    /** Blue Fish **/
    vtkSmartPointer<vtkOBJReader> blueFishReader = vtkSmartPointer<vtkOBJReader>::New();                                                           
    blueFishReader->SetFileName("../Models/obj/fish2.obj");                                                            
    blueFishReader->Update(); 

    vtkSmartPointer<vtkCustomMapperP> blueFishMapper = vtkSmartPointer<vtkCustomMapperP>::New();                                                      
    blueFishMapper->SetInputConnection(blueFishReader->GetOutputPort()); 

    vtkSmartPointer<vtkActor> blueFishActor = vtkSmartPointer<vtkActor>::New();

    vtkSmartPointer<vtkProperty> blueFishProp = vtkSmartPointer<vtkProperty>::New();
    blueFishProp->SetDiffuseColor(.011, .103, 1.0);
    blueFishActor->SetMapper(blueFishMapper);
    blueFishActor->SetProperty(blueFishProp);
    blueFishActor->SetPosition(-7, 0, 0);
    blueFishActor->RotateY(90);

    /** Yellow Fish **/
    vtkSmartPointer<vtkOBJReader> yellowFishReader = vtkSmartPointer<vtkOBJReader>::New();                                                           
    yellowFishReader->SetFileName("../Models/obj/fish3.obj");
    yellowFishReader->Update(); 

    vtkSmartPointer<vtkCustomMapperP> yellowFishMapper = vtkSmartPointer<vtkCustomMapperP>::New();                                                      
    yellowFishMapper->SetInputConnection(yellowFishReader->GetOutputPort()); 

    vtkSmartPointer<vtkActor> yellowFishActor = vtkSmartPointer<vtkActor>::New();

    vtkSmartPointer<vtkProperty> yellowFishProp = vtkSmartPointer<vtkProperty>::New();
    yellowFishProp->SetDiffuseColor(1.0, 0.854, 0.0);
    yellowFishActor->SetMapper(yellowFishMapper);
    yellowFishActor->SetProperty(yellowFishProp);
    yellowFishActor->SetPosition(-17, -5, 0);
    yellowFishActor->RotateY(90);

    /** Coral-1 **/
    vtkSmartPointer<vtkOBJReader> coral1Reader = vtkSmartPointer<vtkOBJReader>::New();                                                           
    coral1Reader->SetFileName("../Models/obj/coral1.obj");
    coral1Reader->Update(); 

    vtkSmartPointer<vtkCustomMapperP> coral1Mapper = vtkSmartPointer<vtkCustomMapperP>::New();                                                      
    coral1Mapper->SetInputConnection(coral1Reader->GetOutputPort()); 

    vtkSmartPointer<vtkActor> coral1Actor = vtkSmartPointer<vtkActor>::New();

    vtkSmartPointer<vtkProperty> coral1Prop = vtkSmartPointer<vtkProperty>::New();
    coral1Prop->SetDiffuseColor(1.0, 0.065, 0.865);
    coral1Actor->SetMapper(coral1Mapper);
    coral1Actor->SetProperty(coral1Prop);
    coral1Actor->SetScale(2.5);
    coral1Actor->SetPosition(-9, -10, -17);

    /** Coral-2 **/
    vtkSmartPointer<vtkOBJReader> coral2Reader = vtkSmartPointer<vtkOBJReader>::New();                                                           
    coral2Reader->SetFileName("../Models/obj/coral2.obj");
    coral2Reader->Update(); 

    vtkSmartPointer<vtkCustomMapperP> coral2Mapper = vtkSmartPointer<vtkCustomMapperP>::New();                                                      
    coral2Mapper->SetInputConnection(coral2Reader->GetOutputPort()); 

    vtkSmartPointer<vtkActor> coral2Actor = vtkSmartPointer<vtkActor>::New();

    coral2Actor->SetMapper(coral2Mapper);
    coral2Actor->SetProperty(coral1Prop);
    coral2Actor->SetScale(2.5);
    coral2Actor->SetPosition(-1, -10, -15);
    
    /** Shell **/
    vtkSmartPointer<vtkOBJReader> shellReader = vtkSmartPointer<vtkOBJReader>::New();                                                           
    shellReader->SetFileName("../Models/obj/shell.obj");
    shellReader->Update(); 

    vtkSmartPointer<vtkCustomMapperP> shellMapper = vtkSmartPointer<vtkCustomMapperP>::New();                                                      
    shellMapper->SetInputConnection(shellReader->GetOutputPort()); 

    vtkSmartPointer<vtkActor> shellActor = vtkSmartPointer<vtkActor>::New();

    vtkSmartPointer<vtkProperty> shellProp = vtkSmartPointer<vtkProperty>::New();
    shellProp->SetDiffuseColor(1, .6, 0);
    shellActor->SetMapper(shellMapper);
    shellActor->SetProperty(shellProp);
    shellActor->SetPosition(4, -10, -12);

    /** Leaf-1**/
    vtkSmartPointer<vtkOBJReader> leaf1Reader = vtkSmartPointer<vtkOBJReader>::New();                                                           
    leaf1Reader->SetFileName("../Models/obj/leaf1.obj");
    leaf1Reader->Update(); 

    vtkSmartPointer<vtkCustomMapperP> leaf1Mapper = vtkSmartPointer<vtkCustomMapperP>::New();                                                      
    leaf1Mapper->SetInputConnection(leaf1Reader->GetOutputPort()); 

    vtkSmartPointer<vtkActor> leaf1Actor = vtkSmartPointer<vtkActor>::New();

    vtkSmartPointer<vtkProperty> leaf1Prop = vtkSmartPointer<vtkProperty>::New();
    leaf1Prop->SetDiffuseColor(0.0, 0.8, 0.0);
    leaf1Actor->SetMapper(leaf1Mapper);
    leaf1Actor->SetProperty(leaf1Prop);
    leaf1Actor->SetScale(3.0);
    leaf1Actor->SetPosition(-23, -10, -17);

    /** Leaf-2 **/
    vtkSmartPointer<vtkCustomMapperP> leaf2Mapper = vtkSmartPointer<vtkCustomMapperP>::New();                                                      
    leaf2Mapper->SetInputConnection(leaf1Reader->GetOutputPort()); 

    vtkSmartPointer<vtkActor> leaf2Actor = vtkSmartPointer<vtkActor>::New();

    leaf2Actor->SetMapper(leaf2Mapper);
    leaf2Actor->SetProperty(leaf1Prop);
    leaf2Actor->SetScale(3.0);
    leaf2Actor->SetPosition(-19, -10, -17);

    /** Leaf-3 **/
    vtkSmartPointer<vtkCustomMapperP> leaf3Mapper = vtkSmartPointer<vtkCustomMapperP>::New();                                                      
    leaf3Mapper->SetInputConnection(leaf1Reader->GetOutputPort()); 

    vtkSmartPointer<vtkActor> leaf3Actor = vtkSmartPointer<vtkActor>::New();

    leaf3Actor->SetMapper(leaf3Mapper);
    leaf3Actor->SetProperty(leaf1Prop);
    leaf3Actor->SetScale(2.7);
    leaf3Actor->SetPosition(-15, -10, -12);

    /** Leaf-4 **/
    vtkSmartPointer<vtkCustomMapperP> leaf4Mapper = vtkSmartPointer<vtkCustomMapperP>::New();                                                      
    leaf4Mapper->SetInputConnection(leaf1Reader->GetOutputPort()); 

    vtkSmartPointer<vtkActor> leaf4Actor = vtkSmartPointer<vtkActor>::New();

    leaf4Actor->SetMapper(leaf4Mapper);
    leaf4Actor->SetProperty(leaf1Prop);
    leaf4Actor->SetScale(2.2);
    leaf4Actor->SetPosition(-10, -10, -12);
    leaf4Actor->RotateY(90);

    /** Leaf-5 **/
    vtkSmartPointer<vtkCustomMapperP> leaf5Mapper = vtkSmartPointer<vtkCustomMapperP>::New();                                                      
    leaf5Mapper->SetInputConnection(leaf1Reader->GetOutputPort()); 

    vtkSmartPointer<vtkActor> leaf5Actor = vtkSmartPointer<vtkActor>::New();

    leaf5Actor->SetMapper(leaf5Mapper);
    leaf5Actor->SetProperty(leaf1Prop);
    leaf5Actor->SetScale(2.);
    leaf5Actor->SetPosition(-5.5, -10, -14);
    leaf5Actor->RotateY(45);

    /** Submarine **/
    vtkSmartPointer<vtkOBJReader> submarineReader = vtkSmartPointer<vtkOBJReader>::New();                                                           
    submarineReader->SetFileName("../Models/obj/submarine.obj");
    submarineReader->Update(); 

    vtkSmartPointer<vtkCustomMapperP> submarineMapper = vtkSmartPointer<vtkCustomMapperP>::New();                                                      
    submarineMapper->SetInputConnection(submarineReader->GetOutputPort()); 

    vtkSmartPointer<vtkActor> submarineActor = vtkSmartPointer<vtkActor>::New();

    vtkSmartPointer<vtkProperty> submarineProp = vtkSmartPointer<vtkProperty>::New();
    submarineProp->SetDiffuseColor(0.012, 0.342, 0.01);
    submarineActor->SetMapper(submarineMapper);
    submarineActor->SetProperty(submarineProp);
    submarineActor->SetPosition(13, -9, 8.5);
    submarineActor->SetScale(2);
    submarineActor->RotateX(-15);
    submarineActor->RotateY(-60);

    /** Tree-1 **/
    vtkSmartPointer<vtkOBJReader> tree1Reader = vtkSmartPointer<vtkOBJReader>::New();                                                           
    tree1Reader->SetFileName("../Models/obj/tree1.obj");
    tree1Reader->Update(); 

    vtkSmartPointer<vtkCustomMapperP> tree1Mapper = vtkSmartPointer<vtkCustomMapperP>::New();                                                      
    tree1Mapper->SetInputConnection(tree1Reader->GetOutputPort()); 

    vtkSmartPointer<vtkActor> tree1Actor = vtkSmartPointer<vtkActor>::New();

    vtkSmartPointer<vtkProperty> tree1Prop = vtkSmartPointer<vtkProperty>::New();
    tree1Prop->SetDiffuseColor(0.016, 0.8, 0.035);
    tree1Actor->SetMapper(tree1Mapper);
    tree1Actor->SetScale(2.5);
    tree1Actor->SetProperty(tree1Prop);
    tree1Actor->SetPosition(-21, -10, -17);

    tree1Mapper->displayAxes = false;

    /** Tree-2  **/
    vtkSmartPointer<vtkCustomMapperP> tree2Mapper = vtkSmartPointer<vtkCustomMapperP>::New();                                                      
    tree2Mapper->SetInputConnection(tree1Reader->GetOutputPort()); 

    vtkSmartPointer<vtkActor> tree2Actor = vtkSmartPointer<vtkActor>::New();

    tree2Actor->SetMapper(tree2Mapper);
    tree2Actor->SetScale(2.);
    tree2Actor->SetProperty(tree1Prop);
    tree2Actor->RotateY(45);
    tree2Actor->SetPosition(-16, -10, -15);
    tree2Mapper->displayAxes = false;

    /** Tree-3 **/
    vtkSmartPointer<vtkCustomMapperP> tree3Mapper = vtkSmartPointer<vtkCustomMapperP>::New();                                                      
    tree3Mapper->SetInputConnection(tree1Reader->GetOutputPort()); 

    vtkSmartPointer<vtkActor> tree3Actor = vtkSmartPointer<vtkActor>::New();

    tree3Actor->SetMapper(tree3Mapper);
    tree3Actor->SetScale(1.5);
    tree3Actor->SetProperty(tree1Prop);
    tree3Actor->RotateY(62);
    tree3Actor->SetPosition(-18.5, -10, -11);
    tree3Mapper->displayAxes = false;

    /** TreeSpire-1 **/
    vtkSmartPointer<vtkOBJReader> treeSpire1Reader = vtkSmartPointer<vtkOBJReader>::New();                                                           
    treeSpire1Reader->SetFileName("../Models/obj/treespire.obj");
    treeSpire1Reader->Update(); 

    vtkSmartPointer<vtkCustomMapperP> treeSpire1Mapper = vtkSmartPointer<vtkCustomMapperP>::New();                                                      
    treeSpire1Mapper->SetInputConnection(treeSpire1Reader->GetOutputPort()); 

    vtkSmartPointer<vtkActor> treeSpire1Actor = vtkSmartPointer<vtkActor>::New();

    vtkSmartPointer<vtkProperty> treeSpire1Prop = vtkSmartPointer<vtkProperty>::New();
    treeSpire1Prop->SetDiffuseColor(1.0, 0.0, 0.429);
    treeSpire1Actor->SetMapper(treeSpire1Mapper);
    treeSpire1Actor->SetProperty(treeSpire1Prop);
    treeSpire1Actor->SetPosition(16, -10, -13);
    treeSpire1Actor->SetScale(3);

    /** TreeSpire-2 **/
    vtkSmartPointer<vtkOBJReader> treeSpire2Reader = vtkSmartPointer<vtkOBJReader>::New();                                                           
    treeSpire2Reader->SetFileName("../Models/obj/treespire2.obj");
    treeSpire2Reader->Update(); 

    vtkSmartPointer<vtkCustomMapperP> treeSpire2Mapper = vtkSmartPointer<vtkCustomMapperP>::New();                                                      
    treeSpire2Mapper->SetInputConnection(treeSpire2Reader->GetOutputPort()); 

    vtkSmartPointer<vtkActor> treeSpire2Actor = vtkSmartPointer<vtkActor>::New();

    treeSpire2Actor->SetMapper(treeSpire2Mapper);
    treeSpire2Actor->SetProperty(treeSpire1Prop);
    treeSpire2Actor->SetPosition(22, -10, -15);
    treeSpire2Actor->SetScale(3);

    /** TreeSpire-3 **/
    vtkSmartPointer<vtkCustomMapperP> treeSpire3Mapper = vtkSmartPointer<vtkCustomMapperP>::New();                                                      
    treeSpire3Mapper->SetInputConnection(treeSpire1Reader->GetOutputPort()); 

    vtkSmartPointer<vtkActor> treeSpire3Actor = vtkSmartPointer<vtkActor>::New();

    treeSpire3Actor->SetMapper(treeSpire3Mapper);
    treeSpire3Actor->SetProperty(treeSpire1Prop);
    treeSpire3Actor->SetPosition(18, -10, -8);
    treeSpire3Actor->RotateY(40);
    treeSpire3Actor->SetScale(2.5);

    /** Rock-1 **/
    vtkSmartPointer<vtkOBJReader> rock1Reader = vtkSmartPointer<vtkOBJReader>::New();                                                           
    rock1Reader->SetFileName("../Models/obj/rock1.obj");
    rock1Reader->Update(); 

    vtkSmartPointer<vtkCustomMapperP> rock1Mapper = vtkSmartPointer<vtkCustomMapperP>::New();                                                      
    rock1Mapper->SetInputConnection(rock1Reader->GetOutputPort()); 

    vtkSmartPointer<vtkActor> rock1Actor = vtkSmartPointer<vtkActor>::New();

    vtkSmartPointer<vtkProperty> rock1Prop = vtkSmartPointer<vtkProperty>::New();
    rock1Prop->SetDiffuseColor(.3, .5, .95);
    rock1Actor->SetMapper(rock1Mapper);
    rock1Actor->SetProperty(rock1Prop);
    rock1Actor->SetPosition(-5, -10, -11);
    rock1Actor->SetScale(.75);

    /** Rock-3 **/
    vtkSmartPointer<vtkOBJReader> rock3Reader = vtkSmartPointer<vtkOBJReader>::New();                                                           
    rock3Reader->SetFileName("../Models/obj/rock3.obj");
    rock3Reader->Update(); 

    vtkSmartPointer<vtkCustomMapperP> rock3Mapper = vtkSmartPointer<vtkCustomMapperP>::New();                                                      
    rock3Mapper->SetInputConnection(rock3Reader->GetOutputPort()); 

    vtkSmartPointer<vtkActor> rock3Actor = vtkSmartPointer<vtkActor>::New();

    vtkSmartPointer<vtkProperty> rock3Prop = vtkSmartPointer<vtkProperty>::New();
    rock3Prop->SetDiffuseColor(1, .1, .865);
    rock3Actor->SetMapper(rock3Mapper);
    rock3Actor->SetProperty(rock3Prop);
    rock3Actor->SetPosition(-11, -9, 8);
    rock3Actor->SetScale(.75);

    /** Rock-2 **/
    vtkSmartPointer<vtkOBJReader> rock2Reader = vtkSmartPointer<vtkOBJReader>::New();                                                           
    rock2Reader->SetFileName("../Models/obj/rock2.obj");
    rock2Reader->Update(); 

    vtkSmartPointer<vtkCustomMapperP> rock2Mapper = vtkSmartPointer<vtkCustomMapperP>::New();                                                      
    rock2Mapper->SetInputConnection(rock2Reader->GetOutputPort()); 

    vtkSmartPointer<vtkActor> rock2Actor = vtkSmartPointer<vtkActor>::New();

    vtkSmartPointer<vtkProperty> rock2Prop = vtkSmartPointer<vtkProperty>::New();
    rock2Prop->SetDiffuseColor(0.016, 0.8, 0.035);
    rock2Actor->SetMapper(rock2Mapper);
    rock2Actor->SetProperty(rock2Prop);
    rock2Actor->SetPosition(11, -10, -12);
    rock2Actor->SetScale(.75);

    /** ShellPearl-1 **/
    vtkSmartPointer<vtkOBJReader> shellPearl1Reader = vtkSmartPointer<vtkOBJReader>::New();                                                           
    shellPearl1Reader->SetFileName("../Models/obj/shellwithpearl_white.obj");
    shellPearl1Reader->Update(); 

    vtkSmartPointer<vtkCustomMapperP> shellPearl1Mapper = vtkSmartPointer<vtkCustomMapperP>::New();                                                      
    shellPearl1Mapper->SetInputConnection(shellPearl1Reader->GetOutputPort()); 

    vtkSmartPointer<vtkActor> shellPearl1Actor = vtkSmartPointer<vtkActor>::New();

    shellPearl1Actor->SetMapper(shellPearl1Mapper);
    shellPearl1Actor->SetPosition(-16, -8.5, 7);
    shellPearl1Actor->SetScale(2);
    
    /** ShellPearl-2 **/
    vtkSmartPointer<vtkOBJReader> shellPearl2Reader = vtkSmartPointer<vtkOBJReader>::New();                                                           
    shellPearl2Reader->SetFileName("../Models/obj/shellwithpearl_purple.obj");
    shellPearl2Reader->Update(); 

    vtkSmartPointer<vtkCustomMapperP> shellPearl2Mapper = vtkSmartPointer<vtkCustomMapperP>::New();                                                      
    shellPearl2Mapper->SetInputConnection(shellPearl2Reader->GetOutputPort()); 

    vtkSmartPointer<vtkActor> shellPearl2Actor = vtkSmartPointer<vtkActor>::New();

    vtkSmartPointer<vtkProperty> shellPearl2Prop = vtkSmartPointer<vtkProperty>::New();
    shellPearl2Prop->SetDiffuseColor(1., .5, .0);
    shellPearl2Actor->SetMapper(shellPearl2Mapper);
    shellPearl2Actor->SetProperty(shellPearl2Prop);
    shellPearl2Actor->SetPosition(-16, -9, 7);
    shellPearl2Actor->SetScale(2);

    /** Fish tank floor **/
    vtkSmartPointer<vtkOBJReader> floorReader = vtkSmartPointer<vtkOBJReader>::New();                                                           
    floorReader->SetFileName("../Models/obj/floor.obj");
    floorReader->Update(); 

    vtkSmartPointer<vtkCustomMapperP> floorMapper = vtkSmartPointer<vtkCustomMapperP>::New();                                                      
    floorMapper->SetInputConnection(floorReader->GetOutputPort()); 

    vtkSmartPointer<vtkActor> floorActor = vtkSmartPointer<vtkActor>::New();
    vtkSmartPointer<vtkProperty> floorProp = vtkSmartPointer<vtkProperty>::New();
    floorProp->SetDiffuseColor(0.0,0.0,0.35);
    floorActor->SetMapper(floorMapper);
    floorActor->SetProperty(floorProp);
    floorActor->SetScale(3.5);
    floorActor->SetPosition(0, -10, 0); 

    /** Fish tank backing **/
    vtkSmartPointer<vtkOBJReader> backgroundReader = vtkSmartPointer<vtkOBJReader>::New();
    backgroundReader->SetFileName("../Models/obj/background.obj");
    backgroundReader->Update(); 

    vtkSmartPointer<vtkCustomMapperP> backgroundMapper = vtkSmartPointer<vtkCustomMapperP>::New();                                                      
    backgroundMapper->SetInputConnection(backgroundReader->GetOutputPort()); 

    vtkSmartPointer<vtkActor> backgroundActor = vtkSmartPointer<vtkActor>::New();
    vtkSmartPointer<vtkProperty> backgroundProp = vtkSmartPointer<vtkProperty>::New();
    backgroundProp->SetDiffuseColor(0.0,0.0,0.15);
    backgroundActor->SetMapper(backgroundMapper);
    backgroundActor->SetProperty(backgroundProp);
    backgroundActor->SetScale(3.5);
    backgroundActor->RotateY(90);
    backgroundActor->SetPosition(0, -11, -18); 

    /**** End model reading ****/

    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();

    vtkSmartPointer<vtkRenderWindow> windowRenderer = vtkSmartPointer<vtkRenderWindow>::New();
    windowRenderer->AddRenderer(renderer);
    renderer->SetViewport(0, 0, 1, 1);

    vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    iren->SetRenderWindow(windowRenderer);

    // Add the actors to the renderer, set the background and size.
    renderer->AddActor(goldFishActor);
    renderer->AddActor(floorActor);
    renderer->AddActor(blueFishActor);
    renderer->AddActor(yellowFishActor);
    renderer->AddActor(submarineActor);  
    renderer->AddActor(tree1Actor);
    renderer->AddActor(tree2Actor);
    renderer->AddActor(tree3Actor);
    renderer->AddActor(leaf1Actor);
    renderer->AddActor(leaf2Actor);
    renderer->AddActor(leaf3Actor);
    renderer->AddActor(leaf4Actor);
    renderer->AddActor(leaf5Actor);
    renderer->AddActor(coral1Actor);
    renderer->AddActor(coral2Actor);
    renderer->AddActor(treeSpire1Actor);
    renderer->AddActor(treeSpire2Actor);
    renderer->AddActor(treeSpire3Actor);
    renderer->AddActor(shellPearl1Actor);
    renderer->AddActor(shellPearl2Actor);
    renderer->AddActor(backgroundActor);
    renderer->AddActor(shellActor);
    renderer->AddActor(rock1Actor);
    renderer->AddActor(rock2Actor);
    renderer->AddActor(rock3Actor);
    renderer->SetBackground(0, 0, 0);
    windowRenderer->SetSize(650, 650);

    // Set up the lighting.
    renderer->GetActiveCamera()->SetFocalPoint(0,0,0);
    renderer->GetActiveCamera()->SetPosition(0,0,70);
    renderer->GetActiveCamera()->SetViewUp(0,1,0);
    renderer->GetActiveCamera()->SetClippingRange(20, 120);
    renderer->GetActiveCamera()->SetDistance(170);

    renderer->RemoveAllLights();
    vtkSmartPointer<vtkLight> l = vtkSmartPointer<vtkLight>::New();
    l->SetAmbientColor(0.15, 0.15, 0.15); 
    l->SetPosition(0.0,1.5,1.0);
    l->SetFocalPoint(0,0,0);
    vtkSmartPointer<vtkLight> l2 = vtkSmartPointer<vtkLight>::New();
    l2->SetAmbientColor(0.0, 0.0, 0.5); 
    l2->SetPosition(0.0,0.0,9.9);
    l2->SetFocalPoint(0,0,0);
    renderer->AddLight(l);
    renderer->AddLight(l2);
   
    vtkSmartPointer<vtkInteractorStyleJoystickCamera> style = vtkSmartPointer<vtkInteractorStyleJoystickCamera>::New();
  
    iren->SetInteractorStyle(style); 
    // Start the event loop and invoke an initial render.
    iren->Initialize();

    fish = goldFishMapper; 
    window = windowRenderer;

    iren->Start();

    return EXIT_SUCCESS;
}

