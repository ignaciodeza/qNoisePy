/*
qNoise: A generator of non-Gaussian colored noise
Copyright © 2021, Juan Ignacio Deza
email: ignacio.deza@uwe.ac.uk

Description
qNoise is a non-gaussian colored random noise generator. It is a handy source of
self-correlated noise for a great variety of applications. It depends on two
parameters only: tau for controlling the autocorrelation, and q for controlling
the statistics. This noise tends smoothly  for q = 1 to an  Ornstein-Uhlenbeck
(colored gaussian) noise with autocorrelation tau. for q < 1 it is bounded noise
and it is supra-Gaussian for q > 1. The noise is generated  via a stochastic
differential equation using the Heun method (a second order Runge-Kutta type
integration scheme) and it is implemented as a stand-alone library in c++. It
Useful as input for numerical simulations, as a source of noise for controlling
experiments using synthetic noise via micro-controllers and for a wide variety
of applications.

Requirements
It is a stand-alone library with no dependencies other than the standard
libraries. Due to it's use of some functions from the <random> library the
library currently works on c++11 or higher only. This should be OK for most Macs
and new Linux systems. In some older systems it is possible that you need to add
`-std=gnu++11` to your compilation flags.

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

#ifndef QNOISEGEN_H
#define QNOISEGEN_H

#include <iostream>
#include <climits>
#include <cstdio>
#include <ctime>
#include <cmath>
#include <random>
#include <chrono>
#include <ctime>
#include <ratio>

class qNoiseGen {
    double potQNoisePrime(double eta, double tau, double q);
    typedef std::chrono::high_resolution_clock myclock;
    myclock::time_point beginning;
    unsigned seed;
    std::mt19937 generator;
    std::normal_distribution<double> randNorm;
    std::uniform_real_distribution<double> uniform;

public:
    void seedManual(unsigned UserSeed);
    void seedTimer();
    double gaussWN();
    double orsUhl(double x, double tau, double H);
    double qNoise(double x, double tau, double q, double H, double sqrt_H);
    double qNoiseNorm(double x, double tau, double q, double H, double sqrt_H);

    qNoiseGen() {
        std::normal_distribution<double> randNorm(0.0, 1.0);
        std::uniform_real_distribution<double> uniform(0.0,0.99);
        beginning = myclock::now();
        seedTimer();
    }
};

#endif  // QNOISEGEN_H