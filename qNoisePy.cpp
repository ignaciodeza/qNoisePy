/*
qNoise: A generator of non-Gaussian colored noise
Copyright © 2021, Juan Ignacio Deza
email: ignacio.deza@uwe.ac.uk

Description
qNoise is a non-gaussian colored random noise generator. It is a handy source of self-correlated noise for a great variety of applications. It depends on two parameters only: tau for controlling the autocorrelation, and q for controlling the statistics. This noise tends smoothly  for q = 1 to an  Ornstein-Uhlenbeck (colored gaussian) noise with autocorrelation tau. for q < 1 it is bounded noise and it is supra-Gaussian for q > 1.
The noise is generated  via a stochastic differential equation using the Heun method (a second order Runge-Kutta type integration scheme) and it is implemented as a stand-alone library in c++.
It Useful as input for numerical simulations, as a source of noise for controlling experiments using synthetic noise via micro-controllers and for a wide variety of applications.

Requirements
It is a stand-alone library with no dependencies other than the standard libraries.
Due to it's use of some functions from the <random> library the library currently works on c++11 or higher only.
This should be OK for most Macs and new Linux systems.
In some older systems it is possible that you need to add `-std=gnu++11` to your compilation flags.

Licence
 This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <cstdlib>
#include "qNoise.cpp"
#include <vector>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

// -------------
// pure C++ code
// -------------
qNoiseGen gen;

std::vector<double> qNoise(double tau, double q,int N=10, double H=0.01, int temp_N=-1, bool norm = true) {
    double x; //Value of the noise
    std::vector<double> output(N); //Vector to store the output
    double sqrt_H = sqrt(H);
    
    if (tau < 0) tau = -tau;
    if (tau == 0) tau = H;

    if(temp_N <0)
        temp_N = 2 * tau / H; //transient time. It's a good idea to make it dependent of Tau

    x = gen.gaussWN() / 100; //Initial value, can be anything. Better if it's around zero.
    for (int i = -temp_N; i < N ; i++) {
        if(norm)
            x = gen.qNoiseNorm(x, tau, q, H, sqrt_H); //call to qNoise Normalised
        else  
            x = gen.qNoise(x, tau, q, H, sqrt_H);  //call to qNoise
        if (i >= 0)
            output[i] = x;   //Write to vector for output
    }
    return output;
}

std::vector<double> OrnsteinUhlenbeck(double tau,int N=10, double H=0.01, int temp_N = -1, bool white_noise = false, double ini_cond=0){
    double x; //Value of the noise
    std::vector<double> output(N); //Vector to store the output
    
    if (tau < 0) tau = -tau;
    if (tau == 0) white_noise = true;

    if(white_noise){
        for (int i = 0; i < N ; i++) 
            output[i] = gen.gaussWN();
        return output;
    }

    if(temp_N <0){
        temp_N = 2 * tau / H; //transient time. It's a good idea to make it dependent of Tau
        x = gen.gaussWN() / 100; //Initial value, can be anything. Better if it's around zero.
    }
    else
        x = ini_cond;

    for (int i = -temp_N; i < N ; i++) {
            x = gen.orsUhl(x, tau, H);  //call to Ornstein Uhlenbeck
        if (i >= 0)
            output[i] = x;   //Write to vector for output
    }
    return output;

}


// ----------------
// Python interface
// ----------------

namespace py = pybind11;

// wrap C++ function with NumPy array IO
py::array_t<double> py_qnoise(double tau, double q,int N =10, double H=0.01, int temp_N=0, bool norm = false)
{
  
  // call pure C++ function
  std::vector<double> result_vec = qNoise(tau,q,N,H,temp_N);

  // allocate py::array (to pass the result of the C++ function to Python)
  auto result        = py::array_t<double>(N);
  auto result_buffer = result.request();
  double *result_ptr    = (double *) result_buffer.ptr;

  // copy std::vector -> py::array
  std::memcpy(result_ptr,result_vec.data(),result_vec.size()*sizeof(double));
  return result;
}

py::array_t<double> py_orsUhl(double tau,int N =10, double H=0.01, int temp_N=0, bool white_noise = false, double ini_cond = 0)
{
  
  // call pure C++ function
  std::vector<double> result_vec = OrnsteinUhlenbeck(tau,N,H,temp_N, white_noise,ini_cond);

  // allocate py::array (to pass the result of the C++ function to Python)
  auto result        = py::array_t<double>(N);
  auto result_buffer = result.request();
  double *result_ptr    = (double *) result_buffer.ptr;

  // copy std::vector -> py::array
  std::memcpy(result_ptr,result_vec.data(),result_vec.size()*sizeof(double));
  return result;
}

// wrap as Python module
PYBIND11_MODULE(qNoisePy,m)
{
  m.doc() = "qNoise for Python";
  m.def("qNoise", &py_qnoise, "generates N instances of qNoise with tau and q given" , py::arg("tau"), py::arg("q"), 
  py::arg("N") = 10, py::arg("H") = 0.01, py::arg("temp_N") = 0, py::arg("norm")=false);
    m.def("OrnsteinUhlenbeck", &py_orsUhl, "generates N instances of Ornstein-Uhlenbeck noise with tau given", py::arg("tau"), 
  py::arg("N") = 10, py::arg("H") = 0.01, py::arg("temp_N") = 0, py::arg("white_noise")=false, py::arg("ini_cond")=0);
}
