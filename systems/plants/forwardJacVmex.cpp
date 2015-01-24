#include <mex.h>
#include <iostream>
#include "drakeUtil.h"
#include "RigidBodyManipulator.h"
#include "drakeGeometryUtil.h"
#include "math.h"

using namespace Eigen;
using namespace std;

/*
 * A C version of the geometricJacobian function
 *
 * Call with [J, vIndices] = geometricJacobianmex(model_ptr, base_body_or_frame_ind, end_effector_body_or_frame_ind, expressed_in_body_or_frame_ind);
 */

// TODO: stop copying these functions everywhere and find a good place for them
template <typename DerivedA>
mxArray* eigenToMatlab(const DerivedA &m)
{
 mxArray* pm = mxCreateDoubleMatrix(static_cast<int>(m.rows()),static_cast<int>(m.cols()),mxREAL);
 if (m.rows()*m.cols()>0)
   memcpy(mxGetPr(pm),m.data(),sizeof(double)*m.rows()*m.cols());
 return pm;
}

mxArray* stdVectorToMatlab(const std::vector<int>& vec) {
//  mxArray* pm = mxCreateNumericMatrix(vec.size(), 1, mxINT32_CLASS, mxREAL);
//  if (vec.size() > 0) {
//    memcpy(mxGetPr(pm), vec.data(), sizeof(int) * vec.size());
//  }
//  return pm;

  mxArray* pm = mxCreateDoubleMatrix(static_cast<int>(vec.size()), 1, mxREAL);
  for (int i = 0; i < static_cast<int>(vec.size()); i++) {
    mxGetPr(pm)[i] = (double) vec[i];
  }
  return pm;
}

template<int RowsAtCompileTime, int ColsAtCompileTime>
Eigen::Matrix<double, RowsAtCompileTime, ColsAtCompileTime> matlabToEigen(const mxArray* matlab_array)
{
  const mwSize* size_array = mxGetDimensions(matlab_array);
  Eigen::Matrix<double, RowsAtCompileTime, ColsAtCompileTime> ret(size_array[0], size_array[1]);
  memcpy(ret.data(), mxGetPr(matlab_array), sizeof(double) * ret.size());
  return ret;
}

void mexFunction( int nlhs, mxArray *plhs[],int nrhs, const mxArray *prhs[] ) {

  std::string usage = "Usage [J, dJ] = forwardJacVMex(model_ptr, body_or_frame_ind, points, rotation_type, base_or_frame_ind)";
  if (nrhs != 5) {
    mexErrMsgIdAndTxt("Drake:geometricJacobianmex:WrongNumberOfInputs", usage.c_str());
  }

  if (nlhs > 2) {
    mexErrMsgIdAndTxt("Drake:geometricJacobianmex:WrongNumberOfOutputs", usage.c_str());
  }

  // first get the model_ptr back from matlab
  RigidBodyManipulator *model= (RigidBodyManipulator*) getDrakeMexPointer(prhs[0]);

  int body_or_frame_ind = ((int) mxGetScalar(prhs[1])) - 1; // base 1 to base 0
  auto points = matlabToEigen<SPACE_DIMENSION, Eigen::Dynamic>(prhs[2]);
  int rotation_type = (int) mxGetScalar(prhs[3]);
  int base_or_frame_ind = ((int) mxGetScalar(prhs[4])) - 1; // base 1 to base 0

  int gradient_order = nlhs - 1;
  if (nlhs > 1) {
    auto J = model->forwardJacV<double>(body_or_frame_ind, points, rotation_type, base_or_frame_ind, gradient_order);
    plhs[0] = eigenToMatlab(J.value());
    if (gradient_order > 0) {
      plhs[1] = eigenToMatlab(J.gradient().value());
    }
  }

}
