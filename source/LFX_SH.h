#pragma once

#include "LFX_Math.h"
#include "LFX_Vec3.h"
#include <cmath>
#include <functional>
#include <vector>

namespace LFX {

#define SH_BASIS_COUNT 9

class LightProbeSampler {
public:
    /**
     *  generate one sample from sphere uniformly
     */
    static Float3 uniformSampleSphere(float u1, float u2);

    /**
     *  generate ucount1 * ucount2 samples from sphere uniformly
     */
    static std::vector<Float3> uniformSampleSphereAll(uint32_t sampleCount);

    /**
     *  probability density function of uniform distribution on spherical surface
     */
    static inline float uniformSpherePdf() { return 1.0F / (4.0F * Pi); }
};

/**
 * Spherical Harmonics utility class
 */
class SH {
public:
    using BasisFunction = std::function<float(const Float3& v)>;

    /**
     * recreate a function from sh coefficients
     */
    static Float3 evaluate(const Float3& sample, const std::vector<Float3>& coefficients);

    /**
     * project a function to sh coefficients
     */
    static std::vector<Float3> project(const std::vector<Float3>& samples, const std::vector<Float3>& values);

    /**
     * calculate irradiance's sh coefficients from radiance's sh coefficients directly
     */
    static std::vector<Float3> convolveCosine(const std::vector<Float3>& radianceCoefficients);

    /**
     * return basis function count
     */
    static inline uint32_t getBasisCount() {
        return SH_BASIS_COUNT;
    }

    /**
     * evaluate from a basis function
     */
    static inline float evaluateBasis(uint32_t index, const Float3& sample) {
        assert(index < getBasisCount());
        const auto& func = _basisFunctions[index];

        return func(sample);
    }

private:
    static inline float lambda(int32_t l) {
        return std::sqrtf((4.0F * Pi) / (2.0F * static_cast<float>(l) + 1.0F));
    }

    static inline int32_t toIndex(int32_t l, int32_t m) {
        return l * l + l + m;
    }

    static std::vector<BasisFunction> _basisFunctions;
};

}
