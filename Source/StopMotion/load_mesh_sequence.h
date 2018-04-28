#pragma once

#include <vector>
#include <Eigen/Core>

namespace tyro 
{   
    //bool load_movie()

    bool load_mesh_sequence(const std::vector<std::string>& obj_paths,
                            std::vector<Eigen::MatrixXd>& v_data, 
                            std::vector<Eigen::MatrixXd>& n_data,  
                            Eigen::MatrixXi& f_data, 
                            bool use_igl_loader = true);
}

