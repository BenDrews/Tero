/** \file App.cpp */
#include "YApp.h"
#include "YMesh.h"
#include <iostream>
#include <fstream>
#include <stdio.h>


using namespace std;
// Tells C++ to invoke command-line main() function even on OS X and Win32.
G3D_START_AT_MAIN();

int main(int argc, const char* argv[]) {
    {
        G3DSpecification g3dSpec;
        g3dSpec.audio = false;
        initGLG3D(g3dSpec);
    }

    GApp::Settings settings(argc, argv);

    // Change the window and other startup parameters by modifying the
    // settings class.  For example:
    settings.window.caption = argv[0];

    // Set enable to catch more OpenGL errors
    // settings.window.debugContext     = true;

    // Some common resolutions:
    // settings.window.width            =  854; settings.window.height       = 480;
    // settings.window.width            = 1024; settings.window.height       = 768;
    settings.window.width = 1280; settings.window.height = 720;
    //settings.window.width             = 1920; settings.window.height       = 1080;
    // settings.window.width            = OSWindow::primaryDisplayWindowSize().x; settings.window.height = OSWindow::primaryDisplayWindowSize().y;
    settings.window.fullScreen = false;
    settings.window.resizable = !settings.window.fullScreen;
    settings.window.framed = !settings.window.fullScreen;

    // Set to true for a significant performance boost if your app can't render at 60fps, or if
    // you *want* to render faster than the display.
    settings.window.asynchronous = false;

    settings.hdrFramebuffer.depthGuardBandThickness = Vector2int16(64, 64);
    settings.hdrFramebuffer.colorGuardBandThickness = Vector2int16(0, 0);
    settings.dataDir = FileSystem::currentDirectory();
    settings.screenshotDirectory = "../journal/";

    settings.renderer.deferredShading = true;
    settings.renderer.orderIndependentTransparency = false;

    return App(settings).run();
}


App::App(const GApp::Settings& settings) : GApp(settings) {
}


// Called before the application loop begins.  Load data here and
// not in the constructor so that common exceptions will be
// automatically caught.
void App::onInit() {
    GApp::onInit();
    //setFrameDuration(1.0f / 120.0f);

    // Call setScene(shared_ptr<Scene>()) or setScene(MyScene::create()) to replace
    // the default scene here.

    showRenderingStats = false;

    makeGUI();
    // For higher-quality screenshots:
    // developerWindow->videoRecordDialog->setScreenShotFormat("PNG");
    // developerWindow->videoRecordDialog->setCaptureGui(false);
    developerWindow->cameraControlWindow->moveTo(Point2(developerWindow->cameraControlWindow->rect().x0(), 0));


    //loadScene(
    //    //"G3D Sponza"
    //    "Test Scene" // Load something simple
    //    //developerWindow->sceneEditorWindow->selectedSceneName()  // Load the first scene encountered 
    //);


    /* shared_ptr<ArticulatedModel> test;
     test->createEmpty("ya");

     ArticulatedModel::Part* part(test -> addPart("part",NULL));
     ArticulatedModel::Geometry* geom(test -> addGeometry("geom"));
     ArticulatedModel::Mesh* mesh(test->addMesh("first",part,geom));*/

}



/**helper function that saves palettes to a PNG file from ParseVOX.*/

void App::savePNG(G3D::ParseVOX s) {
    shared_ptr<Image> image(Image::create(256, 1, ImageFormat::RGB32F()));
    for (int i = 0; i < 255; ++i) {
        image->set(Point2int32(i, 0), s.palette[i + 1]);
    }
    image->set(Point2int32(255,0),s.palette[0]);
    image->convert(ImageFormat::RGB8());
    image->save("../data-files/model/"+m_outputName+".png");
}

/**helper function that saves an arbitrary MTL file that links palette and the OBJ file*/

void App::saveMTL(G3D::ParseVOX s) {
    TextOutput myfile("../data-files/model/"+m_outputName+".mtl");
    myfile.printf("newmtl palette \n");
    //don't know what this means
    /*myfile.printf("illium \n");*/
    myfile.printf("Ka 1.000 1.000 1.000 \n");
    myfile.printf("Kd 1.000 1.000 1.000 \n");
    myfile.printf("Ks 0.000 0.000 0.000 \n");
    myfile.printf("map_Kd "+m_outputName+".png");
    myfile.commit();
}







void App::makeGUI() {
    // Initialize the developer HUD
    createDeveloperHUD();

    debugWindow->setVisible(true);
    developerWindow->videoRecordDialog->setEnabled(true);

    debugWindow->pack();
    debugWindow->setRect(Rect2D::xywh(0, 0, (float)window()->width(), debugWindow->rect().height()));

    shared_ptr<GuiWindow> window = GuiWindow::create("Controls", debugWindow->theme(), Rect2D::xywh(1025, 175, 0, 0), GuiTheme::TOOL_WINDOW_STYLE);
    GuiPane* pane = window->pane();
    pane->addLabel("Use WASD keys + right mouse to move");
   
    pane->addTextBox("Output Name", &m_outputName);
    pane->beginRow(); {
        pane->addTextBox("Input VOX", &m_filesource)->setWidth(210);
        pane->addButton("...", [this]() {
            FileDialog::getFilename(m_filesource, "vox", false);
        })->setWidth(30);
    } pane->endRow();
    pane->addButton("Generate", [this]() {
        try {
            message("loading");
            //m_outputName=(String)m_filesource.substr(0,m_filesource.length()-4);
            BinaryInput voxInput(m_filesource, G3D_LITTLE_ENDIAN);
            ParseVOX s;
            s.parse(voxInput);
            
            Mesh mesh;
            
            savePNG(s);
            saveMTL(s);
            
            ArticulatedModel::clearCache();
            UniversalMaterial::clearCache();
            mesh.initFromVoxels(s,m_outputName);
            /*loadScene(
                developerWindow->sceneEditorWindow->selectedSceneName()
            );*/
        }
        catch (...) {
            msgBox("Unable to load the image.", m_filesource);
        }
    });

 
    window->pack();

    window->setVisible(true);
    addWidget(window);
}



void App::message(const String& msg) const {
    renderDevice->clear();
    renderDevice->push2D();
    debugFont->draw2D(renderDevice, msg, renderDevice->viewport().center(), 12,
        Color3::white(), Color4::clear(), GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);
    renderDevice->pop2D();

    // Force update so that we can see the message
    renderDevice->swapBuffers();
}


// This default implementation is a direct copy of GApp::onGraphics3D to make it easy
// for you to modify. If you aren't changing the hardware rendering strategy, you can
// delete this override entirely.
void App::onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& allSurfaces) {
    if (!scene()) {
        if ((submitToDisplayMode() == SubmitToDisplayMode::MAXIMIZE_THROUGHPUT) && (!rd->swapBuffersAutomatically())) {
            swapBuffers();
        }
        rd->clear();
        rd->pushState(); {
            rd->setProjectionAndCameraMatrix(activeCamera()->projection(), activeCamera()->frame());
            drawDebugShapes();
        } rd->popState();
        return;
    }

    GBuffer::Specification gbufferSpec = m_gbufferSpecification;
    extendGBufferSpecification(gbufferSpec);
    m_gbuffer->setSpecification(gbufferSpec);
    m_gbuffer->resize(m_framebuffer->width(), m_framebuffer->height());
    m_gbuffer->prepare(rd, activeCamera(), 0, -(float)previousSimTimeStep(), m_settings.hdrFramebuffer.depthGuardBandThickness, m_settings.hdrFramebuffer.colorGuardBandThickness);

    m_renderer->render(rd, m_framebuffer, scene()->lightingEnvironment().ambientOcclusionSettings.enabled ? m_depthPeelFramebuffer : shared_ptr<Framebuffer>(),
        scene()->lightingEnvironment(), m_gbuffer, allSurfaces);

    // Debug visualizations and post-process effects
    rd->pushState(m_framebuffer); {
        // Call to make the App show the output of debugDraw(...)
        rd->setProjectionAndCameraMatrix(activeCamera()->projection(), activeCamera()->frame());
        drawDebugShapes();
        
        shared_ptr<Entity>& selectedEntity = (notNull(developerWindow) && notNull(developerWindow->sceneEditorWindow)) ? developerWindow->sceneEditorWindow->selectedEntity() : shared_ptr<Entity>();
        scene()->visualize(rd, selectedEntity, allSurfaces, sceneVisualizationSettings(), activeCamera());

        // Post-process special effects
        m_depthOfField->apply(rd, m_framebuffer->texture(0), m_framebuffer->texture(Framebuffer::DEPTH), activeCamera(), m_settings.hdrFramebuffer.depthGuardBandThickness - m_settings.hdrFramebuffer.colorGuardBandThickness);

        m_motionBlur->apply(rd, m_framebuffer->texture(0), m_gbuffer->texture(GBuffer::Field::SS_EXPRESSIVE_MOTION),
            m_framebuffer->texture(Framebuffer::DEPTH), activeCamera(),
            m_settings.hdrFramebuffer.depthGuardBandThickness - m_settings.hdrFramebuffer.colorGuardBandThickness);
    } rd->popState();

    // We're about to render to the actual back buffer, so swap the buffers now.
    // This call also allows the screenshot and video recording to capture the
    // previous frame just before it is displayed.
    if (submitToDisplayMode() == SubmitToDisplayMode::MAXIMIZE_THROUGHPUT) {
        swapBuffers();
    }

    // Clear the entire screen (needed even though we'll render over it, since
    // AFR uses clear() to detect that the buffer is not re-used.)
    rd->clear();

    // Perform gamma correction, bloom, and SSAA, and write to the native window frame buffer
    m_film->exposeAndRender(rd, activeCamera()->filmSettings(), m_framebuffer->texture(0), settings().hdrFramebuffer.colorGuardBandThickness.x + settings().hdrFramebuffer.depthGuardBandThickness.x, settings().hdrFramebuffer.depthGuardBandThickness.x);
}


void App::onAI() {
    GApp::onAI();
    // Add non-simulation game logic and AI code here
}


void App::onNetwork() {
    GApp::onNetwork();
    // Poll net messages here
}


void App::onSimulation(RealTime rdt, SimTime sdt, SimTime idt) {
    GApp::onSimulation(rdt, sdt, idt);

    // Example GUI dynamic layout code.  Resize the debugWindow to fill
    // the screen horizontally.
    debugWindow->setRect(Rect2D::xywh(0, 0, (float)window()->width(), debugWindow->rect().height()));
}


bool App::onEvent(const GEvent& event) {
    // Handle super-class events
    if (GApp::onEvent(event)) { return true; }

    // If you need to track individual UI events, manage them here.
    // Return true if you want to prevent other parts of the system
    // from observing this specific event.
    //
    // For example,
    // if ((event.type == GEventType::GUI_ACTION) && (event.gui.control == m_button)) { ... return true; }
    // if ((event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey::TAB)) { ... return true; }
    // if ((event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == 'p')) { ... return true; }

    if ((event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == 'p')) {
        shared_ptr<DefaultRenderer> r = dynamic_pointer_cast<DefaultRenderer>(m_renderer);
        r->setDeferredShading(!r->deferredShading());
        return true;
    }

    return false;
}


void App::onUserInput(UserInput* ui) {
    GApp::onUserInput(ui);
    (void)ui;
    // Add key handling here based on the keys currently held or
    // ones that changed in the last frame.
}


void App::onPose(Array<shared_ptr<Surface> >& surface, Array<shared_ptr<Surface2D> >& surface2D) {
    GApp::onPose(surface, surface2D);

    // Append any models to the arrays that you want to later be rendered by onGraphics()
}


void App::onGraphics2D(RenderDevice* rd, Array<shared_ptr<Surface2D> >& posed2D) {
    // Render 2D objects like Widgets.  These do not receive tone mapping or gamma correction.
    Surface2D::sortAndRender(rd, posed2D);
}


void App::onCleanup() {
    // Called after the application loop ends.  Place a majority of cleanup code
    // here instead of in the constructor so that exceptions can be caught.
}





//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//IGNORE














 //TextOutput myfile("../data-files/model/culling.obj");
    ////hashtable for the voxels, int is just a random int.
    //Table<int, int> texcoordtable;
    //const Array<ParseVOX::Voxel>& voxelarray = s.voxel;
    ////int l = voxelarray.length();


    //Set<Point3uint8> occupied;
    //for (const ParseVOX::Voxel& voxel : voxelarray) {
    //    occupied.insert(voxel.position);
    //}

    //Array<int> indexArray;

    //for (const ParseVOX::Voxel& voxel : voxelarray) {
    //    Point3uint8 position = voxel.position;
    //    Vector3 vecposition(position.x, position.y, position.z);



    //    if (!occupied.contains(position + Vector3uint8(1, 0, 0))) {
    //        Vector3::Axis axis = Vector3::X_AXIS;
    //        float sign = +1;
    //        addQuad(axis, position, sign, ...);

    //        /*
    //        const int n = vertexArray.size();

    //        vertexArray.append(vecposition + Vector3(1, 0, 0), vecposition + Vector3(1, 1, 0), vecposition + Vector3(1, 1, 1), vecposition + Vector3(1, 0, 1));
    //        for (int a = 0; a < 3; ++a) {
    //            indexArray.append(n + a);
    //            textureCoords.append(Vector2((float(voxel.index) - 0.5f) / 256.0f, 0.5f));
    //            //normalArray.append(Vector3(1, 0, 0));
    //        }
    //        indexArray.append(n);
    //        textureCoords.append(Vector2((float(voxel.index) - 0.5f) / 256.0f, 0.5f));
    //        for (int a = 2; a < 4; ++a) {
    //            indexArray.append(n + a);

    //            //normalArray.append(Vector3(1, 0, 0));
    //        }
    //        */
    //    }
    //    if (!occupied.contains(position + Vector3uint8(-1, 0, 0))) {
    //        const int n = vertexArray.size();

    //        vertexArray.append(vecposition + Vector3(0, 0, 0), vecposition + Vector3(0, 0, 1), vecposition + Vector3(0, 1, 1), vecposition + Vector3(0, 1, 0));

    //        for (int a = 0; a < 3; ++a) {
    //            indexArray.append(n + a);
    //            textureCoords.append(Vector2((float(voxel.index) - 0.5f) / 256.0f, 0.5f));
    //            //normalArray.append(Vector3(1, 0, 0));
    //        }
    //        indexArray.append(n);
    //        textureCoords.append(Vector2((float(voxel.index) - 0.5f) / 256.0f, 0.5f));
    //        for (int a = 2; a < 4; ++a) {
    //            indexArray.append(n + a);

    //            //normalArray.append(Vector3(1, 0, 0));
    //        }
    //    }
    //    if (!occupied.contains(position + Vector3uint8(0, 1, 0))) {
    //        const int n = vertexArray.size();

    //        vertexArray.append(vecposition + Vector3(0, 1, 0), vecposition + Vector3(0, 1, 1), vecposition + Vector3(1, 1, 1), vecposition + Vector3(1, 1, 0));


    //        for (int a = 0; a < 3; ++a) {
    //            indexArray.append(n + a);
    //            textureCoords.append(Vector2((float(voxel.index) - 0.5f) / 256.0f, 0.5f));
    //            //normalArray.append(Vector3(1, 0, 0));
    //        }
    //        indexArray.append(n);
    //        textureCoords.append(Vector2((float(voxel.index) - 0.5f) / 256.0f, 0.5f));
    //        for (int a = 2; a < 4; ++a) {
    //            indexArray.append(n + a);

    //            //normalArray.append(Vector3(1, 0, 0));
    //        }

    //    }
    //    if (!occupied.contains(position + Vector3uint8(0, -1, 0))) {

    //        const int n = vertexArray.size();

    //        vertexArray.append(vecposition + Vector3(0, 0, 0), vecposition + Vector3(1, 0, 0), vecposition + Vector3(1, 0, 1), vecposition + Vector3(0, 0, 1));


    //        for (int a = 0; a < 3; ++a) {
    //            indexArray.append(n + a);
    //            textureCoords.append(Vector2((float(voxel.index) - 0.5f) / 256.0f, 0.5f));
    //            //normalArray.append(Vector3(1, 0, 0));
    //        }
    //        indexArray.append(n);
    //        textureCoords.append(Vector2((float(voxel.index) - 0.5f) / 256.0f, 0.5f));
    //        for (int a = 2; a < 4; ++a) {
    //            indexArray.append(n + a);

    //            //normalArray.append(Vector3(1, 0, 0));
    //        }
    //    }
    //    if (!occupied.contains(position + Vector3uint8(0, 0, 1))) {
    //        const int n = vertexArray.size();

    //        vertexArray.append(vecposition + Vector3(0, 0, 1), vecposition + Vector3(1, 0, 1), vecposition + Vector3(1, 1, 1), vecposition + Vector3(0, 1, 1));

    //        for (int a = 0; a < 3; ++a) {
    //            indexArray.append(n + a);
    //            textureCoords.append(Vector2((float(voxel.index) - 0.5f) / 256.0f, 0.5f));
    //            //normalArray.append(Vector3(1, 0, 0));
    //        }
    //        indexArray.append(n);
    //        textureCoords.append(Vector2((float(voxel.index) - 0.5f) / 256.0f, 0.5f));
    //        for (int a = 2; a < 4; ++a) {
    //            indexArray.append(n + a);

    //            //normalArray.append(Vector3(1, 0, 0));
    //        }

    //    }
    //    if (!occupied.contains(position + Vector3uint8(0, 0, -1))) {
    //        const int n = vertexArray.size();

    //        vertexArray.append(vecposition + Vector3(0, 0, 0), vecposition + Vector3(0, 1, 0), vecposition + Vector3(1, 1, 0), vecposition + Vector3(1, 0, 0));

    //        for (int a = 0; a < 3; ++a) {
    //            indexArray.append(n + a);
    //            textureCoords.append(Vector2((float(voxel.index) - 0.5f) / 256.0f, 0.5f));
    //            //normalArray.append(Vector3(1, 0, 0));
    //        }
    //        indexArray.append(n);
    //        textureCoords.append(Vector2((float(voxel.index) - 0.5f) / 256.0f, 0.5f));
    //        for (int a = 2; a < 4; ++a) {
    //            indexArray.append(n + a);

    //            //normalArray.append(Vector3(1, 0, 0));
    //        }

    //    }
    //}
    //Welder::Settings settings;
    //Welder::weld(vertexArray, textureCoords, normalArray, indexArray, settings);
    //myfile.printf("\n# material\nmtllib culling.mtl\nusemtl palette \n");


    ////MAP vertex,normal,texture, to reduce redundant values
    //Table<Vector3, int> vertexMap;
    //Table<Vector3, int> normalMap;
    //Table<Vector2, int> textureMap;
    //for (const Vector3& position : vertexArray) {
    //    if (!vertexMap.containsKey(position)) {
    //        myfile.printf("v %f %f %f \n", position.x, position.z, position.y);
    //        vertexMap.set(position, int(vertexMap.size()));
    //    }
    //}
    //for (const Vector2& texture : textureCoords) {

    //    if (!textureMap.containsKey(texture)) {
    //        myfile.printf("vt %f %f \n", texture.x, texture.y);
    //        textureMap.set(texture, int(textureMap.size()));
    //       
    //    }
    //}
    //for (const Vector3& normal : normalArray) {

    //    if (!normalMap.containsKey(normal)) {
    //        myfile.printf("vn %f %f %f \n", normal.x,normal.z, normal.y);
    //        normalMap.set(normal, int(normalMap.size()));
    //    }
    //}

    ////Printing
    //int size = indexArray.size();
    //for (int i = 0; i < size; i += 3) {

    //    myfile.printf("f %d/%d/%d %d/%d/%d %d/%d/%d \n",
    //        vertexMap.get(vertexArray[indexArray[i] ])+1,
    //        textureMap.get(textureCoords[indexArray[i] ])+1,
    //        normalMap.get(normalArray[indexArray[i]])+1,

    //        vertexMap.get(vertexArray[indexArray[i + 2]])+1,
    //        textureMap.get(textureCoords[indexArray[i + 2]])+1,
    //        normalMap.get(normalArray[indexArray[i + 2]])+1,

    //        vertexMap.get(vertexArray[indexArray[i + 1] ])+1,
    //        textureMap.get(textureCoords[indexArray[i + 1]])+1,
    //        normalMap.get(normalArray[indexArray[i + 1] ])+1

    //        
    //    );
    //}
    //myfile.commit();
  


    ///////////////////////////////////////////////////////


    //remove excess voxels using hashtable
    //for (int i = 0; i < l; ++i) {
    //    const ParseVOX::Voxel& voxel = voxelarray[i];
    //    Point3uint8 position = voxel.position;
    //    if (occupied.contains(Vector3uint8(position.x, position.y, position.z + 1)) &&
    //        occupied.contains(Vector3uint8(position.x + 1, position.y, position.z)) &&
    //        occupied.contains(Vector3uint8(position.x, position.y, position.z - 1)) &&
    //        occupied.contains(Vector3uint8(position.x - 1, position.y, position.z)) &&
    //        occupied.contains(Vector3uint8(position.x, position.y + 1, position.z)) &&
    //        occupied.contains(Vector3uint8(position.x, position.y - 1, position.z))) {
    //        voxelarray.remove(i);
    //        --i;
    //        --l;
    //    }
    //}
    //myfile.printf("\n# material\n mtllib culling.mtl\n usemtl palette \n");
    ////print vertex normals
    //myfile.printf("\n# normals\n");
    //myfile.printf("vn -1 0 0\nvn 1 0 0\nvn 0 0 1\nvn 0 0 -1\nvn 0 -1 0\nvn 0 1 0\n");

    ////print texture coordinates
    ////count the number of texcoords
    //int count = 1;
    //myfile.printf("\n# texcoords\n");
    //for (int i = 0; i < l; ++i) {
    //    ParseVOX::Voxel testvoxel = voxelarray[i];
    //    //convert unint to int
    //    int index = testvoxel.index;
    //    if (!texcoordtable->containsKey(index)) {
    //        texcoordtable->set(index, count);
    //        ++count;
    //        float texture = (((float)index) - 0.5f) / 256.0f;
    //        myfile.printf("vt %f 0.5\n", texture);
    //    }
    //}

    ////print vertex coordinates
    //myfile.printf("\n# verts\n");
    //for (int i = 0; i < l; ++i) {
    //    voxel = voxelarray[i];
    //    Point3uint8 position = voxel.position;
    //    //1
    //    myfile.printf("v %d %d %d \n", position.x, position.z, position.y);
    //    //2
    //    myfile.printf("v %d %d %d \n", position.x + 1, position.z, position.y);
    //    //3
    //    myfile.printf("v %d %d %d \n", position.x + 1, position.z, position.y + 1);
    //    //4
    //    myfile.printf("v %d %d %d \n", position.x, position.z, position.y + 1);
    //    //5
    //    myfile.printf("v %d %d %d \n", position.x, position.z + 1, position.y);
    //    //6
    //    myfile.printf("v %d %d %d \n", position.x + 1, position.z + 1, position.y);
    //    //7
    //    myfile.printf("v %d %d %d \n", position.x + 1, position.z + 1, position.y + 1);
    //    //8
    //    myfile.printf("v %d %d %d \n", position.x, position.z + 1, position.y + 1);
    //}
    ////print surface coordinates and using hashtable to eliminate unused ones
    //myfile.printf("\n# faces\n");
    //for (int i = 0; i < l; ++i) {
    //    voxel = voxelarray[i];
    //    Point3uint8 position = voxel.position;
    //    int texindex = voxel.index;
    //    int texcoindex = texcoordtable->get(texindex);
    //    if (!table->containsKey(Vector3uint8(position.x, position.y, position.z + 1))) {
    //        myfile.printf("f %d/%d/6 %d/%d/6 %d/%d/6 %d/%d/6 \n", 5 + 8 * i, texcoindex, 8 + 8 * i, texcoindex, 7 + 8 * i, texcoindex, 6 + 8 * i, texcoindex);
    //    }
    //    if (!table->containsKey(Vector3uint8(position.x + 1, position.y, position.z))) {
    //        myfile.printf("f %d/%d/2 %d/%d/2 %d/%d/2 %d/%d/2 \n", 6 + 8 * i, texcoindex, 7 + 8 * i, texcoindex, 3 + 8 * i, texcoindex, 2 + 8 * i, texcoindex);
    //    }
    //    if (!table->containsKey(Vector3uint8(position.x, position.y, position.z - 1))) {
    //        myfile.printf("f %d/%d/5 %d/%d/5 %d/%d/5 %d/%d/5 \n", 2 + 8 * i, texcoindex, 3 + 8 * i, texcoindex, 4 + 8 * i, texcoindex, 1 + 8 * i, texcoindex);
    //    }
    //    if (!table->containsKey(Vector3uint8(position.x - 1, position.y, position.z))) {
    //        myfile.printf("f %d/%d/1 %d/%d/1 %d/%d/1 %d/%d/1 \n", 1 + 8 * i, texcoindex, 4 + 8 * i, texcoindex, 8 + 8 * i, texcoindex, 5 + 8 * i, texcoindex);
    //    }
    //    if (!table->containsKey(Vector3uint8(position.x, position.y + 1, position.z))) {
    //        myfile.printf("f %d/%d/3 %d/%d/3 %d/%d/3 %d/%d/3 \n", 8 + 8 * i, texcoindex, 4 + 8 * i, texcoindex, 3 + 8 * i, texcoindex, 7 + 8 * i, texcoindex);
    //    }
    //    if (!table->containsKey(Vector3uint8(position.x, position.y - 1, position.z))) {
    //        myfile.printf("f %d/%d/4 %d/%d/4 %d/%d/4 %d/%d/4 \n", 6 + 8 * i, texcoindex, 2 + 8 * i, texcoindex, 1 + 8 * i, texcoindex, 5 + 8 * i, texcoindex);
    //    }
    //}


    //myfile.commit();

//}


//
//void App::culling2(G3D::ParseVOX s) {
//    TextOutput myfile("../data-files/model/culling.obj");
//    //bad upper bound
//    shared_ptr<Table<G3D::Point3uint8, int>> table(new G3D::Table<G3D::Point3uint8, int>());
//    Array<ParseVOX::Voxel> testarray = s.voxel;
//    ParseVOX::Voxel testvoxel;
//    int l = testarray.length();
//    for (int i = 0; i < l; ++i) {
//        testvoxel = s.voxel[i];
//        Point3uint8 position = testvoxel.position;
//        table->set(position, 1);
//    }
//    for (int i = 0; i < l; ++i) {
//        testvoxel = testarray[i];
//        Point3uint8 position = testvoxel.position;
//        if (table->containsKey(Vector3uint8(position.x, position.y, position.z + 1)) &&
//            table->containsKey(Vector3uint8(position.x + 1, position.y, position.z)) &&
//            table->containsKey(Vector3uint8(position.x, position.y, position.z - 1)) &&
//            table->containsKey(Vector3uint8(position.x - 1, position.y, position.z)) &&
//            table->containsKey(Vector3uint8(position.x, position.y + 1, position.z)) &&
//            table->containsKey(Vector3uint8(position.x, position.y - 1, position.z))) {
//            testarray.remove(i);
//            --i;
//            --l;
//        }
//    }
//
//
//    myfile.printf("vn -1 0 0\nvn 1 0 0\nvn 0 0 1\nvn 0 0 -1\nvn 0 -1 0\nvn 0 1 0\n");
//
//    for (int i = 0; i < l; ++i) {
//        testvoxel = testarray[i];
//        Point3uint8 position = testvoxel.position;
//        //1
//        myfile.printf("v %d %d %d \n", position.x, position.z, position.y);
//        //2
//        myfile.printf("v %d %d %d \n", position.x + 1, position.z, position.y);
//        //3
//        myfile.printf("v %d %d %d \n", position.x + 1, position.z, position.y + 1);
//        //4
//        myfile.printf("v %d %d %d \n", position.x, position.z, position.y + 1);
//        //5
//        myfile.printf("v %d %d %d \n", position.x, position.z + 1, position.y);
//        //6
//        myfile.printf("v %d %d %d \n", position.x + 1, position.z + 1, position.y);
//        //7
//        myfile.printf("v %d %d %d \n", position.x + 1, position.z + 1, position.y + 1);
//        //8
//        myfile.printf("v %d %d %d \n", position.x, position.z + 1, position.y + 1);
//    }
//    for (int i = 0; i < l; ++i) {
//        testvoxel = testarray[i];
//        Point3uint8 position = testvoxel.position;
//
//        myfile.printf("f %d//6 %d//6 %d//6 %d//6 \n", 5 + 8 * i, 8 + 8 * i, 7 + 8 * i, 6 + 8 * i);
//
//        myfile.printf("f %d//2 %d//2 %d//2 %d//2 \n", 6 + 8 * i, 7 + 8 * i, 3 + 8 * i, 2 + 8 * i);
//
//        myfile.printf("f %d//5 %d//5 %d//5 %d//5 \n", 2 + 8 * i, 3 + 8 * i, 4 + 8 * i, 1 + 8 * i);
//
//        myfile.printf("f %d//1 %d//1 %d//1 %d//1 \n", 1 + 8 * i, 4 + 8 * i, 8 + 8 * i, 5 + 8 * i);
//
//        myfile.printf("f %d//3 %d//3 %d//3 %d//3 \n", 8 + 8 * i, 4 + 8 * i, 3 + 8 * i, 7 + 8 * i);
//
//        myfile.printf("f %d//4 %d//4 %d//4 %d//4 \n", 6 + 8 * i, 2 + 8 * i, 1 + 8 * i, 5 + 8 * i);
//
//    }
//    /*TextOutput testfile("../data-files/model/test.txt");
//    for (int i = 0; i < 256; ++i) {
//        Color4unorm8 color(s.palette[i]);
//
//        testfile.printf("v %d %d %d \n",color.r,color.g,color.b);
//    }*/
//    myfile.commit();
//    /*testfile.commit();*/
//}