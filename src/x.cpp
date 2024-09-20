#include <tinynurbs/tinynurbs.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <iostream>
#include <iomanip>

#include "Demangle.h"

using namespace std;

tinynurbs::Curve3f getNonrationalBezierCurve() {
    tinynurbs::Curve3f crv;
    crv.control_points = {
                            glm::vec3(-1, 0, 0),
                            glm::vec3(0, 1, 0),
                            glm::vec3(1, 0, 0)
                        };
    crv.knots = {0, 0, 0, 1, 1, 1};
    crv.degree = 2;
    return crv;
}

int main(int argc, char *argv[])
{
	auto crv = getNonrationalBezierCurve();
	cout << setprecision(3) << fixed;
	for (float t = 0.0; t < 1.0; t += .025) {
		auto pt = tinynurbs::curvePoint(crv, t);
        auto derv = tinynurbs::curveDerivatives(crv, 1, t)[0];
		cout << setw(8) << pt.x
            << ' ' << setw(8) << pt.y
            << ' ' << setw(8) << pt.z
            << "  ## " << setw(8) << derv.x
            << ' ' << setw(8) << derv.y
            << ' ' << setw(8) << derv.z
            << endl;
        // cout << demangle(derv) << endl;
	}
}

