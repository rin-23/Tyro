#include "bshape_app.h"
#include "RAEnginePrerequisites.h"
#include "RAES2TextOverlay.h"
#include "RAFont.h"
#include "RAVisibleSet.h"
#include <stdio.h>
#include <functional>
#include "Wm5APoint.h"
#include "Wm5Vector2.h"
#include "Wm5Vector4.h"
#include "TyroIGLMesh.h"
#include <random>
#include "load_mesh_sequence.h"
#include "TyroIGLMesh.h"
#include <filesystem/path.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "OpenFaceTexture.h"
#include "RATextureBuffer.h"
#include <boost/filesystem.hpp>
#include <iostream>
#include "utils.h"

using namespace boost::filesystem;

using namespace std;

using Wm5::APoint;

#define MESH_COLOR Eigen::Vector3d(109/255.0f, 150/255.0f, 144/255.0f)

namespace tyro
{   
    void console_render_to_image(App* app, const std::vector<std::string>& args) 
    {   
        BshapeApp* bapp =(BshapeApp*) app;
        if (args.size()!=1) return;
        auto name = filesystem::path(args[0]);
        
        //TODOD DRAW SHIT HERE
        //bapp->DrawMeshes();

        // Poll for and process events
        bapp->m_tyro_window->GetGLContext()->swapBuffers();

        //make sure everything was drawn
        glFlush();
        glFinish();
        GL_CHECK_ERROR;
        
        int v_width, v_height;
        bapp->m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        u_int8_t* texture = (u_int8_t*) malloc(4*v_width *v_height);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glReadPixels(0, 0, v_width, v_height, GL_RGBA, GL_UNSIGNED_BYTE, texture);
        GL_CHECK_ERROR;

        cv::Mat image1(v_height, v_width, CV_8UC4, texture);
        cv::Mat image2, image3;
        cv::cvtColor(image1, image2, CV_RGBA2BGR);
        cv::flip(image2, image3, 0);
        
        const std::string RENDER_IMGS_PATH = "/home/rinat/Workspace/FacialManifoldSource/data_anim/images";

        auto fldr = filesystem::path(RENDER_IMGS_PATH)/name;
        if (!fldr.exists())
            filesystem::create_directory(fldr); 
        
        fldr = fldr/filesystem::path("_1.png");

        cv::imwrite(fldr.str(), image3);
        free(texture);
    }

    void console_render_to_images(App* app, const std::vector<std::string>& args) 
    {
        if (args.size()!=1) 
            return;
        
        
        BshapeApp* bapp =(BshapeApp*) app;
        auto name = filesystem::path(args[0]);
        const std::string RENDER_IMGS_PATH = "/home/rinat/Workspace/FacialManifoldSource/data_anim/images";
        auto fldr = filesystem::path(RENDER_IMGS_PATH)/name;
        if (!fldr.exists()) 
        {
            filesystem::create_directory(fldr); 
        }
        
        int v_width, v_height;
        bapp->m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        u_int8_t* texture = (u_int8_t*) malloc(4*v_width *v_height);

        for (int frame = 0; frame < bapp->mCurAnimation.getNumFrames(); ++frame) 
        { 
            bapp->m_frame = frame;
            /*DRAW SHIT HERE*/
            //bapp->DrawMeshes();

            // Poll for and process events
            bapp->m_tyro_window->GetGLContext()->swapBuffers();

            //make sure everything was drawn
            glFlush();
            glFinish();
            GL_CHECK_ERROR;
            
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glReadPixels(0, 0, v_width, v_height, GL_RGBA, GL_UNSIGNED_BYTE, texture);
            GL_CHECK_ERROR;

            cv::Mat image1(v_height, v_width, CV_8UC4, texture);
            cv::Mat image2, image3;
            cv::cvtColor(image1, image2, CV_RGBA2BGR);
            cv::flip(image2, image3, 0);
                        
            auto fldr_to_write = fldr/filesystem::path(std::to_string(frame)+".png");

            cv::imwrite(fldr_to_write.str(), image3);
        }
        free(texture);
    }

    BshapeApp::BshapeApp()
    :
    App(),
    m_frame(0),
    m_state(BshapeApp::State::None),
    m_need_rendering(false),
    m_frame_overlay(nullptr),
    m_show_wire(true)
    {}

    BshapeApp::~BshapeApp() 
    {}
    
    int BshapeApp::Setup(int width, int height) 
    {   
        App::Setup(width, height);

        int v_width, v_height;
        m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        Wm5::Vector4i viewport(0, 0, v_width, v_height);

        m_state = BshapeApp::State::Launched;
        m_need_rendering = true;
     
        m_timeline = new Timeline(24, 300);
        m_timeline->frameChanged = [&](Timeline& timeline, int frame)->void 
        {   
            //RA_LOG_INFO("Frame Change BEGIN");
            m_frame = frame;
            m_need_rendering = true;
            //glfwPostEmptyEvent();
            //RA_LOG_INFO("Frame Change END");
        };
     
        register_console_function("render_to_image", console_render_to_image, ""); 
        register_console_function("render_to_images", console_render_to_images, ""); 

        // show current number of frames
        ES2FontSPtr font = FontManager::GetSingleton()->GetSystemFontOfSize12();
        
        m_frame_overlay = ES2TextOverlay::Create(std::string("Not initialized text"), 
                                                 Wm5::Vector2f(0, 0), 
                                                 font, 
                                                 Wm5::Vector4f(0,0,1,1), 
                                                 viewport);
        m_frame_overlay->SetTranslate(Wm5::Vector2i(-viewport[2]/2 + 50,-viewport[3]/2 + 50));

        m_dist1 = ES2TextOverlay::Create(std::string("Min distance to manifold"), 
                                        Wm5::Vector2f(0, 0), 
                                        font, 
                                        Wm5::Vector4f(0,0,1,1), 
                                        viewport);
        m_dist1->SetTranslate(Wm5::Vector2i(-viewport[2]/2 + 250,-viewport[3]/2 + 150));

        m_dist2 = ES2TextOverlay::Create(std::string("Min distance to manifold"), 
                                         Wm5::Vector2f(0, 0), 
                                         font, 
                                         Wm5::Vector4f(0,0,1,1), 
                                         viewport);

        m_dist2->SetTranslate(Wm5::Vector2i(viewport[2]/2 - 600,-viewport[3]/2 + 150));

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(m_tyro_window->GetGLFWWindow(), true);
        ImGui_ImplOpenGL3_Init("#version 400");

        //load all bshapes and neuteral expression
        std::string modelPath("/home/rinat/Workspace/tyro/apps/face_bshapes/resources/facemodel");
        const bool load_serialized = true;
        if (load_serialized) 
        {
            mFaceModel.deserialize(modelPath);
        }
        else
        {
            mFaceModel.InitDefault();
            mFaceModel.serialize(modelPath);
        }
        
        //load neuteral expression
        Eigen::MatrixXd V, N;
        Eigen::MatrixXi F;
        mFaceModel.getExpression(V, F, N);
        RENDER.mesh = IGLMesh::Create(V, F, N, MESH_COLOR);
        RENDER.mesh->Update(true);
        RENDER.mesh2 = IGLMesh::Create(V, F, N, MESH_COLOR);
        Wm5::Transform tr;
        tr.SetTranslate(Wm5::APoint(-1.5*RENDER.mesh->WorldBoundBox.GetRadius(), 0, 0));
        RENDER.mesh2->LocalTransform = tr * RENDER.mesh2->LocalTransform;  
        RENDER.mesh2->Update(true);
        update_camera();

        // create a video stream
        // m_camera_texture = OpenFaceTexture::Create();

        //setup torch model
        mTorchModelUp.Init("/home/rinat/Workspace/FacialManifoldSource/data_anim/traced.pth");
        mTorchModelLow.Init("/home/rinat/Workspace/FacialManifoldSource/data_anim/traced.pth");

        //find joystick
        int present = m_tyro_window->JoystickConnected(); 
        RA_LOG_INFO("Found joystik %i", present);

        //init kd tree
        std::string csv_file = "/home/rinat/Workspace/FacialManifoldSource/data_anim/clusters/augemented.txt";
        tyro::csvToVector(csv_file, MOTION_DATA);
        mTree.InitWithData(MOTION_DATA);

        //init gamepad
        mGamepad.Init();
    }
    
    int BshapeApp::Launch()
    {   
        //load animation
        //loadAnimation(ANIM_LIST[0]);
                
        m_state = BshapeApp::State::LoadedModel;

        while (!m_tyro_window->ShouldClose())
        {   
            m_gl_rend->ClearScreen();

            m_tyro_window->ProcessUserEvents();

            GamepadExample();    
            //DrawUI();
            //loadOpenFace(); 
            //loadFrame(m_frame);
            //FetchGamepadInputs();
            //ComputeDistanceToManifold();
            //DrawMeshes();
            //ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // Draw console
            if (show_console) 
            {
                glUseProgram(0);
                m_console.display(2);
            }
            
            m_tyro_window->GetGLContext()->swapBuffers();
            m_need_rendering = false;             
            //m_tyro_window->Wait();
        }

        // Cleanup
        //ImGui_ImplOpenGL3_Shutdown();
        //ImGui_ImplGlfw_Shutdown();
        //ImGui::DestroyContext();

	    return 0;
    }

    // method is called once per render frame.
    // all update/draw logic fog gamepad example goes here
    void BshapeApp::GamepadExample() 
    {   
        /*******  FETCH GAMEPAD INPUTS ********/
        std::map<std::string, double> axes; 
        std::map<std::string, bool> btns;
        m_tyro_window->JoystickAxes(axes);
        m_tyro_window->JoystickButtons(btns);
        mGamepad.UpdateFrame(axes, btns);

        std::vector<double> low_values_denoised;
        //mTorchModelLow.Compute(mGamepad.low_values, low_values_denoised);

        std::vector<double> up_values_denoised;
        //mTorchModelUp.Compute(mGamepad.up_values, up_values_denoised); 

        /****** RENDER MESH *******/
        VisibleSet vis_set;
        
        mFaceModel.setWeights(mGamepad.low_bnames, mGamepad.low_values);
        mFaceModel.setWeights(mGamepad.up_bnames, mGamepad.up_values);
        Eigen::MatrixXd V, N;
        Eigen::MatrixXi F;
        mFaceModel.getExpression(V, F, N);
        RENDER.mesh->UpdateData(V, F, N, MESH_COLOR);
        RENDER.mesh->Update(true);

        //mFaceModel.setWeights(low_bnames, low_values_denoised);
        //mFaceModel.setWeights(up_bnames, up_values_denoised);
        Eigen::MatrixXd V2, N2;
        Eigen::MatrixXi F2;
        mFaceModel.getExpression(V2, F2, N2);
        RENDER.mesh2->UpdateData(V2, F2, N2, MESH_COLOR);
        RENDER.mesh2->Update(true);
        

        vis_set.Insert(RENDER.mesh.get());
        vis_set.Insert(RENDER.mesh2.get());
        //vis_set.Insert(m_dist1.get());
        //vis_set.Insert(m_dist2.get());
  
        m_gl_rend->RenderVisibleSet(&vis_set, m_camera);      

        //DrawUI();
        //loadOpenFace(); 
        //loadFrame(m_frame);
        //FetchGamepadInputs();
        //ComputeDistanceToManifold();
        //DrawMeshes();
        //ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    int BshapeApp::LaunchOffScreen(const std::string& csv_file, const std::string& out_fldr) 
    {   
        RA_LOG_INFO("Launching the app in offscreen mode");
 
        //setup windowshapes
        m_tyro_window = new Window();
        m_tyro_window->InitOffscreen(1600,1200);
                
        //setup renderer
        m_gl_rend = new ES2Renderer(m_tyro_window->GetGLContext());
        m_gl_rend->SetClearColor(Wm5::Vector4f(150/255.0, 150/255.0, 150/255.0, 1));

        int v_width, v_height;
        m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        Wm5::Vector4i viewport(0, 0, v_width, v_height);
        m_camera = new iOSCamera(Wm5::APoint(0,0,0), 1.0, 1.0, 2, viewport, true);

        //load all bshapes and neuteral expression
        //mFaceModel.setNeuteralMesh(NEUT);
        //mFaceModel.setBshapes(BSHAPES_MAP); // ORDER IS VERY IMPORTANT to match BSHAPES in arig.py
        std::string modelPath("/home/rinat/Workspace/tyro/apps/face_bshapes/resources/facemodel");
        mFaceModel.deserialize(modelPath);
        
        //load neuteral expression
        Eigen::MatrixXd V, N;
        Eigen::MatrixXi F;
        mFaceModel.getExpression(V, F, N);
        RENDER.mesh = IGLMesh::Create(V, F, N, MESH_COLOR);
        RENDER.mesh->Update(true);
        mCurAnimation.readPandasCsv(csv_file, 0);
        assert(mCurAnimation.getNumFrames() > 0);

        update_camera();

        u_int8_t* texture = (u_int8_t*) malloc(4*v_width *v_height);
        auto out_path = filesystem::path(out_fldr);

        for (int i=0;i<mCurAnimation.getNumFrames(); ++i) 
        {
            m_gl_rend->ClearScreen();

            std::vector<double> W;
            std::vector<std::string> A;
            mCurAnimation.getWeights(i, A, W);
            mFaceModel.setWeights(A, W);
            
            Eigen::MatrixXd V, N;
            Eigen::MatrixXi F;
            mFaceModel.getExpression(V, F, N);
            
            RENDER.mesh->UpdateData(V, F, N, MESH_COLOR);
            RENDER.mesh->Update(true);
            
            VisibleSet vis_set;
            vis_set.Insert(RENDER.mesh.get());
            m_gl_rend->RenderVisibleSet(&vis_set, m_camera);       

            // Poll for and process events
            m_tyro_window->GetGLContext()->swapBuffers();

            //make sure everything was drawn
            glFlush();
            glFinish();
            GL_CHECK_ERROR;
            
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glReadPixels(0, 0, v_width, v_height, GL_RGBA, GL_UNSIGNED_BYTE, texture);
            GL_CHECK_ERROR;

            cv::Mat image1(v_height, v_width, CV_8UC4, texture);
            cv::Mat image2, image3;
            cv::cvtColor(image1, image2, CV_RGBA2BGR);
            cv::flip(image2, image3, 0);
                        
            auto fldr_to_write = out_path/filesystem::path(std::to_string(i)+".png");

            cv::imwrite(fldr_to_write.str(), image3);   
        }

        free(texture);
        
	    return 0;
    }

    /*
    void App::DrawUI()
    {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        //if (show_demo_window)
        //ImGui::ShowDemoWindow();

        if (ImGui::TreeNode("Animations"))
        {
            // ShowHelpMarker("This is a more standard looking tree with selectable nodes.\nClick to select, CTRL+Click to toggle, click on arrows or double-click to open.");
            // static bool align_label_with_current_x_position = false;
            // ImGui::Checkbox("Align label with current X position)", &align_label_with_current_x_position);
            // ImGui::Text("Hello!");
            // if (align_label_with_current_x_position)
            // ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());

            ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize()*3); // Increase spacing to differentiate leaves from expanded contents.
            for (int i = 0; i < ANIM_LIST.size(); i++)
            {
                // Disable the default open on single-click behavior and pass in Selected flag according to our selection state.
                ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ((file_selected == i) ? ImGuiTreeNodeFlags_Selected : 0);
                // Leaf: The only reason we have a TreeNode at all is to allow selection of the leaf. Otherwise we can use BulletText() or TreeAdvanceToLabelPos()+Text().
                node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
                auto p = filesystem::path(ANIM_LIST[i]);
                ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, "%i %s", i, p.basename().c_str());
                if (ImGui::IsItemClicked()) 
                {
                    file_selected = i;
                    RA_LOG_INFO("file selected %i", file_selected);      
                }      
            }

            ImGui::PopStyleVar();
            if(ImGui::Button("Load Animation"))
            {   
                if (file_selected >= 0)
                    RA_LOG_INFO("load animation %s", ANIM_LIST[file_selected].c_str());
                
                loadAnimation(ANIM_LIST[file_selected]);
                m_frame = 0;
            }
            //if (align_label_with_current_x_position)
            //    ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
            ImGui::TreePop();
        }
        
        //ImGui::End();
        //ImGui::Begin("Animations", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        // ImGui::Text("Hello from another window!");
        //ImGui::End();
        
        ImGui::Render();
    }
    */
    /*
    void BshapeApp::DrawMeshes() 
    {
        //RA_LOG_INFO("RENDER BEGIN");
        VisibleSet vis_set;

        //m_camera_texture->showFrame();
        //vis_set.Insert(m_camera_texture.get());
        // update face geometry
        //for (int i=0;i < low_bnames.size(); ++i) 
        //{
            //RA_LOG_INFO("%s, %f",low_bnames[i].data(), low_values[i]);
            //RA_LOG_INFO("Denoised %s, %f",low_bnames[i].data(), low_values_denoised[i]);
        //}

        mFaceModel.setWeights(low_bnames, low_values);
        mFaceModel.setWeights(up_bnames, up_values);
        Eigen::MatrixXd V, N;
        Eigen::MatrixXi F;
        mFaceModel.getExpression(V, F, N);
        RENDER.mesh->UpdateData(V, F, N, MESH_COLOR);
        RENDER.mesh->Update(true);

        //mFaceModel.setWeights(low_bnames, low_values_denoised);
        //mFaceModel.setWeights(up_bnames, up_values_denoised);
        //Eigen::MatrixXd V2, N2;
        //Eigen::MatrixXi F2;
        //mFaceModel.getExpression(V2, F2, N2);
        //RENDER.mesh2->UpdateData(V2, F2, N2, MESH_COLOR);
        //RENDER.mesh2->Update(true);
        
        vis_set.Insert(RENDER.mesh.get());
        vis_set.Insert(RENDER.mesh2.get());
        
        // update timeline text
        std::string fstr = std::string("Frame ") + std::to_string(m_frame) + std::string("/") + std::to_string(mCurAnimation.getNumFrames());
        m_frame_overlay->SetText(fstr);
        vis_set.Insert(m_frame_overlay.get());
        vis_set.Insert(m_dist1.get());
        vis_set.Insert(m_dist2.get());

        // update camera 
        //if (m_update_camera) 
       // {
       //     update_camera();
       //     m_update_camera = false;
       // }
        
        m_gl_rend->RenderVisibleSet(&vis_set, m_camera);       
    }
    */
    /*
    void App::loadFrame(int frame) 
    {
        std::vector<double> W;
        std::vector<std::string> A;
        mCurAnimation.getWeights(frame, A, W);
        mFaceModel.setWeights(A, W);
    }
    */

    /*
    void App::loadOpenFace() 
    {   
        std::vector<std::string> names; 
        std::vector<double> values;
        m_camera_texture->getAUs(names, values);
        
        low_bnames.clear();
        low_values.clear(); 
                
        //for (auto i : lower_face_bshape_index) 
        for (auto& a : OpengFaceAUs) 
        {
            low_bnames.push_back(a);
            low_values.push_back(0.0);
        }
        std::cout << low_bnames << "\n";

        //std::vector<std::string> A; 
        //std::vector<double> W;
        
        //iterate over available values for OpenFace
        for (int i=0; i<names.size();++i) 
        {
            auto k = names[i];
            if (OPENFACE_TO_BSHAPES_MAP.count(k)) //check if we support those attributes 
            {
                auto v = OPENFACE_TO_BSHAPES_MAP.at(k);
                for (auto& au : v) // iterate though the list of mapped values, usually one but sometimes 2
                {
                    //A.push_back(au);
                    //get and scale the value
                    double scaled = values[i]/5.0;
                    if (scaled <= 0.1) scaled = 0.0; //reject noise
                    assert(scaled <= 1.0 && scaled >=0);

                    //construct lower values vector to feed to NN
                    auto it = std::find(low_bnames.begin(), low_bnames.end(), au);
                    assert(it != low_bnames.end());
                    int index = std::distance(low_bnames.begin(), it);
                    low_values[index] = scaled;
                    //RA_LOG_INFO("%s, %0.3f", k.c_str(), scaled);
                    
                    //W.push_back(scaled);
                }
            }
        }

        low_values_denoised.clear();
        mTorchModel.Compute(low_values, low_values_denoised); 
    }
    */
    
    // void BshapeApp::FetchGamepadInputs() 
    // {   
    //     std::map<std::string, double> axes; 
    //     std::map<std::string, bool> btns;
    //     m_tyro_window->JoystickAxes(axes);
    //     m_tyro_window->JoystickButtons(btns);
        
    //     mGamepad.UpdateFrame(axes, btns);

    //     low_values_denoised.clear();
    //     //mTorchModelLow.Compute(mGamepad.low_values, low_values_denoised);

    //     up_values_denoised.clear();
    //     //mTorchModelUp.Compute(mGamepad.up_values, up_values_denoised); 

    //     /*
    //     low_bnames.clear();
    //     low_values.clear();

    //     for (auto& a : OpengFaceAUs)
    //     {
    //         low_bnames.push_back(a);
    //         low_values.push_back(0.0);
    //     } 

    //     for (auto const& a : GAMEPAD_TO_AUS) 
    //     {   
    //         auto value = fabs(axes[a.first]);
    //         for (auto& au : a.second) 
    //         {   
    //             auto it = std::find(low_bnames.begin(), low_bnames.end(), au);
    //             assert(it != low_bnames.end());
    //             int index = std::distance(low_bnames.begin(), it);
    //             low_values[index] = value;
    //         }
    //     }
    //     */
    // }

    /*
    void App::ComputeDistanceToManifold() 
    {   
        double c_dist1, c_dist2;
        mTree.FindClosest(low_values_denoised, c_dist1);
        mTree.FindClosest(low_values, c_dist2);
        RA_LOG_INFO("Distances denoised  %f  noisy %f", c_dist1, c_dist2);
        std::string fstr1 = std::string("(denoised) Min distance to manifold ") + std::to_string(c_dist1);
        std::string fstr2 = std::string("(1to1) Min distance to manifold ") + std::to_string(c_dist2);
        m_dist1->SetText(fstr1);
        m_dist2->SetText(fstr2);
    }
    */
    
    /*
    void App::loadAnimation(const std::string& name) 
    {
        mCurAnimation.readPandasCsv(name, 0);
        //mCurAnimation.setCustomAttrs(mFaceModel.getBnames());
        //m_timeline->Stop();
        m_timeline->SetFrameRange(mCurAnimation.getNumFrames()-1);
        m_timeline->SetFrame(0);
    }
    */

    /*
    void App::render() 
    {   
        //RA_LOG_INFO("NEED RENDERING");
        m_need_rendering = true;
    }
    */

    void BshapeApp::update_camera() 
    {
        //setup camera
        AxisAlignedBBox WorldBoundBox = RENDER.mesh->WorldBoundBox;
        if (RENDER.mesh2)
            WorldBoundBox.Merge(RENDER.mesh2->WorldBoundBox);
        
        Wm5::APoint world_center = WorldBoundBox.GetCenter();
        float radius = std::abs(WorldBoundBox.GetRadius()*1.5);
        //Wm5::APoint world_center = Wm5::APoint::ORIGIN;
        //float radius = 1;
        int v_width, v_height;
        m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        Wm5::Vector4i viewport(0, 0, v_width, v_height);
        float aspect = (float)v_width/v_height;
        
        if (m_camera)
            delete m_camera;
        
        m_camera = new iOSCamera(world_center, radius, aspect, 2, viewport, true);
    }

    void BshapeApp::key_pressed(Window& window, unsigned int key, int modifiers) 
    {   
        //RA_LOG_INFO("Key pressed %c", key);
        
        if (key == '`') 
        {   
            //RA_LOG_INFO("Pressed %c", key);
            show_console = !show_console;
            render();
            return;
        }

        if (show_console) 
        {  
           m_console.keyboard(key);
           render();
           return;
        }
        else 
        {
            if (key == 'p') 
            {
                if (m_timeline->state == Timeline::State::Running)
                    m_timeline->Pause();
                else
                    m_timeline->Start();
            }
            else if (key == ']') //next frame
            {
                m_timeline->NextFrame();
                render();
            }
            else if (key == '[')
            {
                m_timeline->PrevFrame();
                render();
            }            
        }
    }
}