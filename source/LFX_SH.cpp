#include "LFX_SH.h"

namespace LFX {

Float3 LightProbeSampler::uniformSampleSphere(float u1, float u2) {
    float z = 1.0F - 2.0F * u1;
    float r = std::sqrtf(std::max(0.0F, 1.0F - z * z));
    float phi = 2.0F * Pi * u2;

    float x = r * std::cosf(phi);
    float y = r * std::sinf(phi);

    return Float3(x, y, z);
}

std::vector<Float3> LightProbeSampler::uniformSampleSphereAll(uint32_t uCount1, uint32_t uCount2) {
    assert(uCount1 > 0U && uCount2 > 0U);

    std::vector<Float3> samples;
    const auto uDelta1 = 1.0F / static_cast<float>(uCount1);
    const auto uDelta2 = 1.0F / static_cast<float>(uCount2);

    for (auto i = 0U; i < uCount1; i++) {
        const auto u1 = (static_cast<float>(i) + 0.5F) * uDelta1;

        for (auto j = 0U; j < uCount2; j++) {
            const auto u2 = (static_cast<float>(j) + 0.5F) * uDelta2;
            const auto sample = uniformSampleSphere(u1, u2);
            samples.push_back(sample);
        }
    }

    return samples;
}

std::vector<SH::BasisFunction> SH::_basisFunctions = {
    [](const Float3& v) -> float { return 0.282095F; },                             // 0.5F * std::sqrtf(InvPi)
    [](const Float3& v) -> float { return 0.488603F * v.y; },                       // 0.5F * std::sqrtf(3.0F * InvPi) * v.y
    [](const Float3& v) -> float { return 0.488603F * v.z; },                       // 0.5F * std::sqrtf(3.0F * InvPi) * v.z
    [](const Float3& v) -> float { return 0.488603F * v.x; },                       // 0.5F * std::sqrtf(3.0F * InvPi) * v.x
    [](const Float3& v) -> float { return 1.09255F * v.y * v.x; },                  // 0.5F * std::sqrtf(15.0F * InvPi) * v.y * v.x
    [](const Float3& v) -> float { return 1.09255F * v.y * v.z; },                  // 0.5F * std::sqrtf(15.0F * InvPi) * v.y * v.z
    [](const Float3& v) -> float { return 0.946175F * (v.z * v.z - 1.0F / 3.0F); }, // 0.75F * std::sqrtf(5.0F * InvPi) * (v.z * v.z - 1.0F / 3.0F)
    [](const Float3& v) -> float { return 1.09255F * v.z * v.x; },                  // 0.5F * std::sqrtf(15.0F * InvPi) * v.z * v.x
    [](const Float3& v) -> float { return 0.546274F * (v.x * v.x - v.y * v.y); },   // 0.25F * std::sqrtf(15.0F * InvPi) * (v.x * v.x - v.y * v.y)
};

Float3 SH::evaluate(const Float3& sample, const std::vector<Float3>& coefficients) {
    Float3 result{0.0F, 0.0F, 0.0F};

    const auto size = coefficients.size();
    for (auto i = 0; i < size; i++) {
        const Float3& c = coefficients[i];
        result += c * evaluateBasis(i, sample);
    }

    return result;
}

std::vector<Float3> SH::project(const std::vector<Float3>& samples, const std::vector<Float3>& values) {
    assert(samples.size() > 0 && samples.size() == values.size());

    // integral using Monte Carlo method
    const auto basisCount = getBasisCount();
    const auto sampleCount = samples.size();
    const auto scale = 1.0F / (LightProbeSampler::uniformSpherePdf() * static_cast<float>(sampleCount));

    std::vector<Float3> coefficients;
    coefficients.reserve(basisCount);

    for (auto i = 0U; i < basisCount; i++) {
        Float3 coefficient{0.0F, 0.0F, 0.0F};

        for (auto k = 0; k < sampleCount; k++) {
            coefficient += values[k] * evaluateBasis(i, samples[k]);
        }

        coefficient *= scale;
        coefficients.push_back(coefficient);
    }

    return coefficients;
}

std::vector<Float3> SH::convolveCosine(const std::vector<Float3>& radianceCoefficients) {
    static const float COSTHETA[3] = {0.8862268925F, 1.0233267546F, 0.4954159260F};    
    const auto lmax = 2;

    std::vector<Float3> irradianceCoefficients;

    for (auto l = 0; l <= lmax; l++) {
        for (auto m = -l; m <= l; m++) {
            auto i = toIndex(l, m);
            Float3 coefficient = lambda(l) * COSTHETA[l] * radianceCoefficients[i];
            irradianceCoefficients.push_back(coefficient);
        }
    }

    return irradianceCoefficients;
}

}
