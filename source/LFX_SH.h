#pragma once

#include <functional>
#include <cmath>
#include <vector>
#include "LFX_Vec3.h"
#include "LFX_Math.h"

namespace LFX {

#define SH_BASIS_FAST_COUNT   4
#define SH_BASIS_NORMAL_COUNT 9

enum class LightProbeQuality {
    Fast = 0,   // 4 basis functions of L0 & L1
    Normal = 1, // 9 basis functions of L0 & L1 & L2
};

class LightProbeSampler {
public:
    /**
     *  generate one sample from sphere uniformly
     */
    static Float3 uniformSampleSphere(float u1, float u2);

    /**
     *  generate ucount1 * ucount2 samples from sphere uniformly
     */
    static std::vector<Float3> uniformSampleSphereAll(uint32_t uCount1, uint32_t uCount2);

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
    static Float3 evaluate(LightProbeQuality quality, const Float3& sample, const std::vector<Float3>& coefficients);

    /**
     * project a function to sh coefficients
     */
    static std::vector<Float3> project(LightProbeQuality quality, const std::vector<Float3>& samples, const std::vector<Float3>& values);

    /**
     * calculate irradiance's sh coefficients from radiance's sh coefficients directly
     */
    static std::vector<Float3> convolveCosine(LightProbeQuality quality, const std::vector<Float3>& radianceCoefficients);

    /**
     * return band count: lmax = 1 or lmax = 2
     */
    static inline int32_t getBandCount(LightProbeQuality quality) {
        return (quality == LightProbeQuality::Normal ? 2 : 1);
    }

    /**
     * return basis function count
     */
    static inline uint32_t getBasisCount(LightProbeQuality quality) {
        static const uint32_t BASIS_COUNTS[] = {SH_BASIS_FAST_COUNT, SH_BASIS_NORMAL_COUNT};
        return BASIS_COUNTS[static_cast<uint32_t>(quality)];
    }

    /**
     * evaluate from a basis function
     */
    static inline float evaluateBasis(LightProbeQuality quality, uint32_t index, const Float3& sample) {
        assert(index < getBasisCount(quality));
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

