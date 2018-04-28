#include "updateStep.h"
#include <algorithm>    // std::min
#include "utils.h"
#include "labelingStep.h"
#include <Eigen/Dense>
#include <iostream>
#include <string>

using namespace Eigen;

void build_S_Matrix(MatrixXd& S, const VectorXi& S_vec)
{		
	int T = S.cols();
	for (int j = 0; j < S_vec.size(); ++j)
	{
		int i = S_vec(j);
		S(i, j) = 1.0f;
	}
}

void build_G_Matrix(MatrixXd& G) 
{
	int Tminus1 = G.cols();
	int i = 1;
	for (int j = 0; j < Tminus1; ++j)
	{
		G(i - 1, j) = -1;
		G(i, j) = 1;
		i += 1;
	}
}

double computeEnergyBShape(const MatrixXd& F, const MatrixXd& L, const MatrixXd& M, const MatrixXd&G, const MatrixXd& S, double w_s)
{
	MatrixXd F_t = F.transpose();
	MatrixXd L_t = L.transpose();
	MatrixXd G_t = G.transpose();
	MatrixXd S_t = S.transpose();

	MatrixXd E_data = M*F*F_t*M - 2*M*F*S_t*L_t*M + M*L*S*S_t*L_t*M;
	MatrixXd E_smooth = (M*F*G*G_t*F_t*M - 2*M*F*G*G_t*S_t*L_t*M + M*L*S*G*G_t*S_t*L_t*M);

	double E_data_trace = E_data.trace();
	double E_smooth_trace = E_smooth.trace();

	return E_data_trace + w_s*E_smooth_trace;
}

void updateStepBShape(const Eigen::MatrixXd& F_T, const Eigen::MatrixXd& M, const Eigen::VectorXi& S_vec, double w_s, Eigen::MatrixXd& L_T, double& oldEnergy, double& newEnergy)
{
	MatrixXd L = L_T.transpose();
	//printMatrix(L, std::string("Printing L"));
	MatrixXd F = F_T.transpose();
	//printMatrix(F, std::string("Printing F"));
	int LL = L.cols(); //number of labels
	int T = F.cols(); //number of frames
	
	MatrixXd S;
	S.resize(LL, T);
	S.setZero();
	build_S_Matrix(S, S_vec);
	//printf("Printing S");
	//printMatrix(S, std::string("Printing S"));

	MatrixXd G;
	G.resize(T, T - 1);
	G.setZero();
	build_G_Matrix(G);
	//printf("Printing G");
	//printMatrix(G, std::string("Printing G"));

	MatrixXd FG = F*G;
	//printMatrix(FG, std::string("Printing FG"));

	//MatrixXd LS = L * S;
	//MatrixXd FG = F * G;
	//MatrixXd SF_T = S * F.transpose();
	MatrixXd F_t = F.transpose();
	MatrixXd L_t = L.transpose();
	MatrixXd G_t = G.transpose();
	MatrixXd S_t = S.transpose();

	MatrixXd A = S*S_t + w_s*S*G*G_t*S_t;
	MatrixXd C = A + A.transpose();
	MatrixXd B = 2*M*M*F*S_t + 2*w_s*M*M*F*G*G_t*S_t;

	MatrixXd M_inv_sq = (M*M).inverse();
	//printMatrix(M, std::string("Printing M"));
	//printMatrix(M_inv_sq, std::string("Printing M inverse squared"));

	MatrixXd b = B.transpose() * M_inv_sq;

	oldEnergy = computeEnergyBShape(F, L, M, G, S, w_s);
	MatrixXd new_L_T = C.jacobiSvd(ComputeThinU | ComputeThinV).solve(b);
	MatrixXd new_L = new_L_T.transpose();
	newEnergy = computeEnergyBShape(F, new_L, M, G, S, w_s);
	L_T = new_L_T;
}

double computeEnergyVertex(const Eigen::DiagonalMatrix<double, Eigen::Dynamic>& M, const MatrixXd& B, const MatrixXd& W, const Eigen::MatrixXd& D, const MatrixXd&G, const MatrixXd& S, double w_s)
{
	
	double E_data_trace = 
		(M*B*W*W.transpose()*B.transpose()*M
		- 2 *M*B*W*S.transpose()*D.transpose()*B.transpose()*M 
		+ M*B*D*S*S.transpose()*D.transpose()*B.transpose()*M).trace();
	
	double E_smooth_trace =
		(M*B*W*G*G.transpose()*W.transpose()*B.transpose()*M 
		- 2*M*B*W*G*G.transpose()*S.transpose()*D.transpose()*B.transpose()*M
		+ M*B*D*S*G*G.transpose()*S.transpose()*D.transpose()*B.transpose()*M).trace();

	return E_data_trace + w_s * E_smooth_trace;
	

	
	return 0;
}

void updateStepVertex(const Eigen::DiagonalMatrix<double, Eigen::Dynamic>& M, const Eigen::MatrixXd& W_T, const Eigen::MatrixXd& B, Eigen::MatrixXd& D_T, const Eigen::VectorXi& S_vec, double w_s, double& oldEnergy, double& newEnergy)
{
	MatrixXd D = D_T.transpose();
	//printMatrix(L, std::string("Printing L"));
	MatrixXd W = W_T.transpose();
	//printMatrix(W, std::string("Printing F"));
	
	int D_cols = D.cols(); //number of labels in dictionary
	int W_cols = W.cols(); //number of frames
	int B_cols = B.cols(); //number of bshape targets
	
	MatrixXd S;
	S.resize(D_cols, W_cols);
	S.setZero();
	build_S_Matrix(S, S_vec);
	//printMatrix(S, std::string("Printing S"));

	MatrixXd G;
	G.resize(W_cols, W_cols - 1);
	G.setZero();
	build_G_Matrix(G);
	//printMatrix(G, std::string("Printing G"));

	//MatrixXd B_t = B.transpose();

	//MatrixXd B_t_B = B.transpose() * B;
	MatrixXd B_t_B = B.transpose() * M * M * B;

	//MatrixXd W_t = W.transpose();
	//MatrixXd D_t = D.transpose();
	//MatrixXd G_t = G.transpose();
	//MatrixXd S_t = S.transpose();
	
	MatrixXd A = S*S.transpose() + w_s*S*G*G.transpose()*S.transpose();
	//printMatrix(A, std::string("Matrix A"));
	MatrixXd C = 2 * A;
	MatrixXd K = 2*B_t_B*(W*S.transpose() + w_s*W*G*G.transpose()*S.transpose());


	MatrixXd B_inv_sq = pseudoinverse(B_t_B);
	//MatrixXd test = B_inv_sq * B_t_B;
	//printMatrix(test, std::string("LUL"));
	//printMatrix(B_t_B, std::string("printing BTB"));
	//MatrixXd B_inv_sq = (B_t_B).inverse();
	//printMatrix(B, std::string("Printing B"));
	//printMatrix(B_inv_sq, std::string("Printing B inverse squared"));
		
	MatrixXd b = K.transpose()*B_inv_sq;
	oldEnergy = computeEnergyVertex(M, B, W, D, G, S, w_s);
	MatrixXd new_D_T = C.jacobiSvd(ComputeThinU | ComputeThinV).solve(b);
	MatrixXd new_D = new_D_T.transpose();
	newEnergy = computeEnergyVertex(M, B, W, new_D, G, S, w_s);
	D_T = new_D_T;
}

double computeEnergyTRUEVertex(const Eigen::MatrixXd& F, const Eigen::MatrixXd& D, const Eigen::MatrixXd& G, const Eigen::VectorXi& S_vec, double w_s)
{
	/*
	double E_data_trace =
		(-2 * F * S.transpose() * D.transpose() + D * S * S.transpose() * D.transpose()).trace();

	double E_smooth_trace =
		(- 2 * F*G*G.transpose()*S.transpose()*D.transpose()
		+ D*S*G*G.transpose()*S.transpose()*D.transpose()).trace();
	*/

	//Computer data energy term
	int numFrames = F.cols();	
	double dataenergy = 0;
	for (int p = 0; p < numFrames; ++p) 
	{
		int l = S_vec(p);
		double diff = (F.col(p) - D.col(l)).squaredNorm();
		dataenergy += diff;
	}

	double smoothEnergy = 0;
	for (int p = 1; p < numFrames; ++p)
	{
		int l1 =  S_vec(p);
		int l2 = S_vec(p - 1);

		double diff = ((D.col(l1) - D.col(l2)) - (F.col(p)- F.col(p-1))).squaredNorm();
		smoothEnergy += diff;
	}

	double totalenergy = dataenergy  + w_s*smoothEnergy;

	return totalenergy;
}

void updateStepTRUEVertex(const Eigen::MatrixXd& F, Eigen::MatrixXd& D, const Eigen::VectorXi& S_vec, double w_s, double& oldEnergy, double& newEnergy) 
{
	
	int D_cols = D.cols(); //number of labels in dictionary
	int F_cols = F.cols(); //number of frames
	
	MatrixXd S;
	S.resize(D_cols, F_cols);
	S.setZero();
	build_S_Matrix(S, S_vec);
	
	MatrixXd G;
	G.resize(F_cols, F_cols - 1);
	G.setZero();
	build_G_Matrix(G);
	//printMatrix(G, std::string("Printing G"));

	MatrixXd A = S*S.transpose() + w_s*S*G*G.transpose()*S.transpose();
	//printMatrix(A, std::string("Matrix A"));
	MatrixXd C = 2 * A;
	MatrixXd K = 2 * (F*S.transpose() + w_s*F*G*G.transpose()*S.transpose());
	MatrixXd b = K.transpose();
	oldEnergy = computeEnergyTRUEVertex(F, D, G, S_vec, w_s);
	MatrixXd new_D_t = C.jacobiSvd(ComputeThinU | ComputeThinV).solve(b);
	D = new_D_t.transpose();
	newEnergy = computeEnergyTRUEVertex(F, D, G, S_vec, w_s);
	assert(D.cols() == D_cols);
}


