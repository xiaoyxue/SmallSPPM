#pragma once

#include "def.h"
#include "utils.h"
#include "math/linagl.h"
#include "math/ray.h"
#include "sampler/sampler.h"
#include "system/memory.h"

NAMESPACE_BEGIN

class PhaseFunction {
public:
    // PhaseFunction Interface
    virtual ~PhaseFunction() {};
    virtual real p(const Vec3& wo, const Vec3& wi) const = 0;
    virtual real Sample_p(const Vec3& wo, Vec3* wi, const Vec2& u) const = 0;
};


// Media Inline Functions
FORCE_INLINE real PhaseHG(real cosTheta, real g) {
    real denom = 1 + g * g + 2 * g * cosTheta;
    return INV_4PI * (1 - g * g) / (denom * std::sqrt(denom));
}

class Intersection;
class MediumIntersection;
class Medium {
public:
    // Medium Interface
    virtual ~Medium() {}
    virtual Vec3 Tr(const Ray& ray, StateSequence& rand) const = 0;
    virtual Vec3 Sample(const Ray& ray, StateSequence& rand, MemoryArena& arena, MediumIntersection* mi) const = 0;
    virtual Vec3 EquiAngularSampling(const Ray& ray, StateSequence& rand, MemoryArena& arena,
        const Intersection& lightPoint, MediumIntersection* mi) const = 0;
};


// HenyeyGreenstein Declarations
class HenyeyGreenstein : public PhaseFunction {
public:
    // HenyeyGreenstein Public Methods
    HenyeyGreenstein(real g) : g(g) {}
    real p(const Vec3& wo, const Vec3& wi) const;
    real Sample_p(const Vec3& wo, Vec3* wi, const Vec2& sample) const;
private:
    const real g;
};

// MediumInterface Declarations
struct MediumInterface {
    MediumInterface() : inside(nullptr), outside(nullptr) {}
    // MediumInterface Public Methods
    MediumInterface(const Medium* medium) : inside(medium), outside(medium) {}
    MediumInterface(const Medium* inside, const Medium* outside)
        : inside(inside), outside(outside) {}
    bool IsMediumTransition() const { return inside != outside; }
    const Medium* inside, * outside;
};

NAMESPACE_END