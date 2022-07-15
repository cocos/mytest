#include "LFX_ILPathTrace.h"
#include "LFX_ILBakerSampling.h"
#include "LFX_World.h"

namespace LFX { namespace ILBaker {

	PathTraceResult ILBaker::PathTrace(const PathTraceParams& params, PathTraceFunc func, Random& rand, bool& hitSky)
	{
		Ray ray;
		ray.orig = params.rayStart;
		ray.dir = params.rayDir;

		PathTraceResult result;
		Float3 throughput = Float3(1.0f, 1.0f, 1.0f);
		Float3 irrThroughput = Float3(1.0f, 1.0f, 1.0f);

		// Keep tracing paths until we reach the specified max
		const int maxPathLength = params.maxPathLength;
		for (; result.pathLen <= maxPathLength || maxPathLength == -1; ++result.pathLen) {
#if 0 // Disable russian roulette
			// See if we should randomly terminate this path using Russian Roullete
			const int rouletteDepth = params.RussianRouletteDepth;
			if (pathLength >= rouletteDepth && rouletteDepth != -1) {
				float continueProbability = std::min<float>(params.RussianRouletteProbability, ComputeLuminance(throughput));
				if (randomGenerator.RandomFloat() > continueProbability) {
					break;
				}
				throughput /= continueProbability;
				irrThroughput /= continueProbability;
			}
#endif

			// Set this to true to keep the loop going
			bool continueTracing = false;

			// Check for intersection with the scene
			Float3 rayOrigin = ray.orig;
			Float3 rayDir = ray.dir;

			Contact contact;
			if (World::Instance()->GetScene()->RayCheck(contact, ray, params.rayLen, LFX_TERRAIN | LFX_MESH)) {
				// We hit a triangle in the scene
				if (result.pathLen == maxPathLength) {
					// There's no point in continuing anymore, since none of our scene surfaces are emissive.
					break;
				}

				// back facing
				if (contact.backFacing) {
					break;
				}

				Material* mtl = (Material*)contact.mtl;
				Vertex vtx = contact.vhit;
				Mat3 tangentToWorld;
				tangentToWorld.SetXBasis(vtx.Tangent);
				tangentToWorld.SetYBasis(vtx.Binormal);
				tangentToWorld.SetZBasis(vtx.Normal);

				Float3 diffuse;
				if (!func(diffuse, params, vtx, mtl, false)) {
					break;
				}

				float lenSq = (vtx.Position - ray.orig).lenSqr();
				result.radiance += diffuse * throughput;
				result.irradiance += (diffuse / lenSq) * irrThroughput;

				// Pick a new path, using MIS to sample both our diffuse and specular BRDF's
				if (1) {
					// Randomly select if we should sample our diffuse BRDF, or our specular BRDF
					Float2 brdfSample = params.sampleSet->BRDF();
					if (result.pathLen > 1) {
						brdfSample = rand.RandomFloat2();
					}

					//Float3 v = Float3::Normalize(rayOrigin - hitSurface.position);

					// We're sampling the diffuse BRDF, so sample a cosine-weighted hemisphere
					Float3 sampleDir;
					sampleDir = SampleCosineHemisphere(brdfSample.x, brdfSample.y);
					sampleDir = Float3::Normalize(Mat3::Transform(sampleDir, tangentToWorld));

					if (sampleDir.dot(vtx.Normal) > 0.0f) {
						throughput = throughput * mtl->Diffuse * params.diffuseScale;
						irrThroughput = irrThroughput * Pi;

						// Generate the ray for the new path
						ray.orig = vtx.Position + sampleDir * 0.01f;
						ray.dir = sampleDir;
						
						continueTracing = true;
					}
				}
			}
			else {
				Float3 diffuse;
				if (func(diffuse, params, Vertex(), nullptr, true)) {
					result.radiance += params.skyRadiance * throughput;
					result.irradiance += params.skyRadiance * irrThroughput;
				}
				hitSky = true;
			}

			if (continueTracing == false) break;
		}

		return result;
	}

}}