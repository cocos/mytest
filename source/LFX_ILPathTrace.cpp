#include "LFX_ILPathTrace.h"
#include "LFX_ILBakerSampling.h"
#include "LFX_World.h"

namespace LFX { namespace ILBaker {

	PathTraceResult ILBaker::PathTrace(const PathTraceParams& params, PathTraceFunc func, Random& rand, bool& hitSky)
	{
		PathTraceResult result;

		Ray ray;
		ray.orig = params.rayStart;
		ray.dir = params.rayDir;

		// Keep tracing paths until we reach the specified max
		float throughput = 1.0f;
		Entity* traceEntity = params.entity;
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
			Contact contact;
			if (World::Instance()->GetScene()->RayCheck(contact, ray, params.rayLen, LFX_TERRAIN | LFX_MESH)) {
				traceEntity = contact.entity;

				// back facing
				if (!contact.facing) {
					break;
				}

				Vertex vtx = contact.vhit;
				Material* mtl = (Material*)contact.mtl;

				Float3 diffuse;
				if (!func(diffuse, params, vtx, mtl, false)) {
					break;
				}

				float lenSq = (vtx.Position - ray.orig).lenSqr();
				//throughput *= 1.0f / (lenSq + 1.0f);
				throughput *= 1.0f / Pi;
				result.color += (diffuse * throughput);

				// Pick a new path, using MIS to sample both our diffuse and specular BRDF's
				if (1) {
					Mat3 tangentToWorld;
					tangentToWorld.SetXBasis(vtx.Tangent);
					tangentToWorld.SetYBasis(vtx.Binormal);
					tangentToWorld.SetZBasis(vtx.Normal);

					Float2 sample;
#if 0
					// Randomly select if we should sample our diffuse BRDF, or our specular BRDF
					if (result.pathLen <= 1) {
						sample = params.sampleSet->BRDF();
					}
					else {
						sample = rand.RandomFloat2();
					}
#else
					sample = rand.RandomFloat2();
#endif

					//Float3 v = Float3::Normalize(rayOrigin - hitSurface.position);

					// We're sampling the diffuse BRDF, so sample a cosine-weighted hemisphere
					Float3 sampleDir;
					sampleDir = SampleCosineHemisphere(sample);
					sampleDir = Float3::Normalize(Mat3::Transform(sampleDir, tangentToWorld));
					if (sampleDir.dot(vtx.Normal) > 0.0f) {
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
					result.color += diffuse * throughput;
				}
				hitSky = true;
			}

			if (continueTracing == false) break;
		}

		return result;
	}

}}