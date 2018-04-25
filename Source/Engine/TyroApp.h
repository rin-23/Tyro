#pragma once

#include "TyroWindow.h"
#include "RAES2Renderer.h"
#include "RAiOSCamera.h"
#include "Console.h"
#include <vector>
#include <Eigen/Core>
#include "TyroTimeLine.h"
#include <atomic>

namespace tyro
{
    class App 
    {
    public:

        enum State {None, Launched, LoadedModel, PlayingAnimation};
        enum SelectionPrimitive {Vertex, Faces, Edges};
        enum SelectionMethod {OneClick, Square};
        enum SelectionType {Select, Deselect};

        App();
        
        ~App();
        
        int Launch();

        void load_blobby();
        void load_oldman();
        void load_bunny();
        void compute_average();
        void compute_deformation();
        void compute_deformation2();
        void save_selected_faces(const std::string& filename);
        void save_selected_verticies(const std::string& filename);
        void load_selected_faces(const std::string& filename);
        void load_selected_verticies(const std::string& filename); 
        void set_sel_primitive(App::SelectionPrimitive sel_state);
        void set_sel_method(App::SelectionMethod sel_state);
        void save_mesh_sequence_with_selected_faces(const std::string& folder, const std::string& filename);
        void clear_all_selection();
        void invert_face_selection();
        void align_all_models();
        void align_all_models(int vid, Eigen::Vector3d offset);
        
        State m_state;
        SelectionPrimitive m_sel_primitive;
        SelectionMethod m_sel_method;
    private:
        Timeline* m_timeline;
        Window* m_tyro_window;
        ES2Renderer* m_gl_rend;
        iOSCamera* m_camera;
        
        void mouse_down(Window& window, int button, int modifier);
        void mouse_up(Window& window, int button, int modifier);
        void mouse_move(Window& window, int mouse_x, int mouse_y);
        void window_resize(Window& window, unsigned int w, unsigned int h);    
        void key_pressed(Window& window, unsigned int key, int modifiers); 
        void key_down(Window& window, unsigned int key, int modifiers); 
        void mouse_scroll(Window& window, float ydelta); 
        
        bool mouse_is_down;
        int gesture_state; 
        int current_mouse_x;
        int current_mouse_y;
        bool show_console;        
        int m_modifier;
        int m_mouse_btn_clicked;
        bool m_computed_deformation;
        bool m_computed_avg;
        Console m_console;

        //square selection
        int m_square_sel_start_x;
        int m_square_sel_start_y;
        

        //IGLMeshSPtr igl_mesh;
        //IGLMeshWireframeSPtr igl_mesh_wire;

        //std::vector<SpatialSPtr> object_list;
        std::vector<SpatialSPtr> ball_list;
        std::atomic<int> m_frame;

        typedef struct MRenderData 
        {
            IGLMeshSPtr org_mesh;
            IGLMeshWireframeSPtr org_mesh_wire;
            bool org_visible = true;

            IGLMeshSPtr avg_mesh;
            IGLMeshWireframeSPtr avg_mesh_wire;
            bool avg_visible = true;

            IGLMeshSPtr dfm_mesh;
            IGLMeshWireframeSPtr dfm_mesh_wire;            
            bool dfm_visible;
        } MRenderData;
        MRenderData render_data;

        typedef struct MAnimation 
        {   
            std::vector<Eigen::MatrixXd> v_data; // Vertex data. 3*num_vert by num_frames. 
            std::vector<Eigen::MatrixXd> n_data; // Normal data. 3*num_vert by num_frames. 
            Eigen::MatrixXi f_data; // Face data.   num_faces by 3
            Eigen::MatrixXd c_data;
            Eigen::MatrixXd avg_v_data;
        } MAnimation;

        MAnimation m_frame_data;
        MAnimation m_frame_deformed_data;

        std::vector<int> vid_list;
        std::vector<int> fid_list;

        void register_console_function(const std::string& name,
                                   const std::function<void(App*, const std::vector<std::string>&)>& con_fun,
                                   const std::string& help_txt);
        void load_mesh_sequence(const std::vector<std::string>& obj_list, bool use_igl_loader = true); 
        void update_camera(const AxisAlignedBBox& WorldBoundBox);
        void render();
        void addSphere(int vid);        
        void removeSpheres(std::vector<int> vids);
        void setFaceColor(int fid, bool selected);
        void selectVertex(Eigen::Vector2f& mouse_pos);
        std::atomic<bool> m_need_rendering;
    };
}