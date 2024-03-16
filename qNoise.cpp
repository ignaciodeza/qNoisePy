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

#include "qNoise.h"

// #define UNIT_TEST
#ifdef UNIT_TEST
#include "unit_tests.cpp"
#endif

// Manual seeding.
void qNoiseGen::seedManual(unsigned UserSeed) {
  seed = UserSeed;
  generator.seed(seed);
}

// Timer seeding
// Note: when using this on multiple threads, call seedManual on each thread
// using different values of seed

void qNoiseGen::seedTimer() {
  // obtain a seed from the timer
  myclock::duration d = myclock::now() - beginning;
  seed = d.count();
  generator.seed(seed);
}

// Gaussian White noise
double qNoiseGen::gaussWN() { return randNorm(generator); }

// Ornstein-Uhlenbeck noise type.
double qNoiseGen::orsUhl(double eta, double tau, double H) {
  return eta * exp(-H / tau) +
         sqrt((1 - exp(-2 / tau * H)) / 2 / tau) * randNorm(generator);
}

/*
 * qNoise Potential derivative. Private Function for use only by qNoise
 * The potential is:
 * \frac{\text{sigma}^2 \log \left(\frac{\text{eta}^2 (q-1)
 * \text{tau}}{\text{sigma}^2}+1\right)}{2 (q-1) \text{tau}} it's derivative is:
 * \frac{\text{eta}}{\frac{\text{eta}^2 (q-1) \text{tau}}{\text{sigma}^2}+1}
 * The /tau is the tau present in the differential equation.
 */
double qNoiseGen::potQNoisePrime(double eta, double tau, double q) {
  return (eta / (1 + (eta * eta * tau * (q - 1)))) / tau;
}

/*qNoise.
 * This functions integrates a differential equation using the Heun Method
 * for q=1 it behaves like the Ornstein Uhlenbeck noise
 * for q<1 it it is defined in a bounded support only (+/- etaCut)
 * for q>1 its statistics are more than gaussian tending (supra-gaussian)
 */
double qNoiseGen::qNoise(double eta, double tau, double q, double H,
                         double sqrt_H = -1) {
  double kHeun, lHeun, differential;
  bool error = false;
  int countError = 0;
  // If the square root of H is provided, it will be used, otherwise
  // calculate it every time the function is invoked.
  if (sqrt_H < 0)
    sqrt_H = sqrt(H);
  // The cut value.
  double etaCut = 1 / sqrt(tau * (1 - q));
  while (1) {
    kHeun = H * potQNoisePrime(eta, tau, q);
    lHeun = sqrt_H * randNorm(generator) / tau;
    differential = -H / 2 *
                       (potQNoisePrime(eta, tau, q) +
                        potQNoisePrime(eta + kHeun + lHeun, tau, q)) +
                   lHeun;
    /*
     * Check if the system is inside the boundary.
     * This is only important when q<1.
     * If the eta + D_eta are outside the bounds, it retries 10 times.
     * If it is still out of bounds, it retries with an OH noise, another 10
     * times This very time step will be Gaussian but with the correct tau. If
     * it is still out of bounds it hard resets it with gaussian noise. The bool
     * error is on place to debug these errors. However it should be a concern
     * only for very low values of q and very high values of tau.
     */
    if ((fabs(eta + differential) > etaCut) || std::isnan(eta + differential)) {
      countError++;
      if (countError > 20) {
        if (error)
          std::cerr << "Out of bounds phase 3: " << eta << "\t" << differential
                    << std::endl;
        return eta/fabs(eta) * etaCut * (0.9 + 0.1 * uniform(generator) );
      }
      if (countError > 10) {
        eta = etaCut * orsUhl(eta, tau, H);
        if (error)
          std::cerr << "Out of bounds phase 2: " << eta << "\t" << differential
                    << std::endl;
      } else {
        if (error)
          std::cerr << "Out of bounds: " << eta << "\t" << differential
                    << std::endl;
      }
    } else {
      return eta + differential;
    }
  }
}

/*qNoiseNorm.
 * This functions works as qNoise but where both tau and the variance of the
 * noise are independent of q to first order. This approximation fails for q ->
 * 5/3, where both variables diverge.
 */
double qNoiseGen::qNoiseNorm(double eta, double tau, double q, double H,
                             double sqrt_H = -1) {
  return qNoise(eta, tau * (5 - 3 * q) / 2, q, H, sqrt_H);
}

extern "C" {
	qNoiseGen* qNoise_new(){ return new qNoiseGen(); }
  double eta,  tau,  q,  H;
	void qNoise_myFunction(qNoiseGen* qn){ qn -> qNoise( eta,  tau,  q,  H); }
}
