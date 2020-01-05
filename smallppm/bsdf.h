#pragma once

#include "linagl.h"
#include "utils.h"
#include "intersection.h"
#include "scalar.h"
#include "transform.h"
#include "sampling.h"
#include "microfacet.h"

NAMESPACE_BEGIN

namespace BSDFCoordinate {
	FORCE_INLINE real CosTheta(const Vec3& w) { return w.z; }

	FORCE_INLINE real Cos2Theta(const Vec3& w) { return w.z * w.z; }

	FORCE_INLINE real AbsCosTheta(const Vec3& w) { return std::abs(w.z); }

	FORCE_INLINE real Sin2Theta(const Vec3& w) {
		return std::max((real)0, (real)1 - Cos2Theta(w));
	}
	FORCE_INLINE real SinTheta(const Vec3& w) { return std::sqrt(Sin2Theta(w)); }

	FORCE_INLINE real TanTheta(const Vec3& w) { return SinTheta(w) / CosTheta(w); }

	FORCE_INLINE real Tan2Theta(const Vec3& w) {
		return Sin2Theta(w) / Cos2Theta(w);
	}

	FORCE_INLINE real CosPhi(const Vec3& w) {
		real sinTheta = SinTheta(w);
		return (sinTheta == 0) ? 1 : Clamp(w.x / sinTheta, -1, 1);
	}

	FORCE_INLINE real SinPhi(const Vec3& w) {
		real sinTheta = SinTheta(w);
		return (sinTheta == 0) ? 0 : Clamp(w.y / sinTheta, -1, 1);
	}

	FORCE_INLINE real Cos2Phi(const Vec3& w) { return CosPhi(w) * CosPhi(w); }

	FORCE_INLINE real Sin2Phi(const Vec3& w) { return SinPhi(w) * SinPhi(w); }

	FORCE_INLINE real CosDPhi(const Vec3& wa, const Vec3& wb) {
		return Clamp(
			(wa.x * wb.x + wa.y * wb.y) / std::sqrt((wa.x * wa.x + wa.y * wa.y) *
			(wb.x * wb.x + wb.y * wb.y)),
			-1, 1);
	}
}

class BSDF {
public:
	BSDF(const Intersection &isect) : 
		n(isect.n), nl(isect.nl), ss(isect.dpdu.Norm()), ts(Cross(nl, ss)) {
		//Vec3 ss, ts;
		//CoordinateSystem(isect.nl, &ss, &ts);
		//LocalToWorld = Transform(
		//	ss.x, ts.x, nl.x, 0,
		//	ss.y, ts.y, nl.y, 0,
		//	ss.z, ts.z, nl.z, 0,
		//	0, 0, 0, 1);
		//WorldToLocal = Inverse(LocalToWorld);
	}
	Vec3 WorldToLocal(const Vec3& v) const {
		return Vec3(Dot(v, ss), Dot(v, ts), Dot(v, nl));
	}
	Vec3 LocalToWorld(const Vec3& v) const {
		return Vec3(ss.x * v.x + ts.x * v.y + nl.x * v.z,
			ss.y * v.x + ts.y * v.y + nl.y * v.z,
			ss.z * v.x + ts.z * v.y + nl.z * v.z);
	}
	virtual ~BSDF(){}
	virtual real Pdf(const Vec3 &wo, const Vec3 &wi) const = 0;
	virtual Vec3 Sample_f(const Vec3 &wo, Vec3 *wi, real *pdf, const Vec3 &rand = Vec3(0, 0, 0)) const = 0;
	virtual Vec3 f(const Vec3 &wo, const Vec3 &wi) const = 0;
	virtual bool IsDelta() const { return false; }
protected:
	const Vec3 n, nl;
	const Vec3 ss, ts;
	static constexpr int MaxBSDFs = 8;
	std::shared_ptr<BxDF> bxdfs[MaxBSDFs];

};

class BxDF {
	virtual ~BxDF(){}
	virtual real Pdf(const Vec3& wo, const Vec3& wi) const = 0;
	virtual Vec3 Sample_f(const Vec3& wo, Vec3* wi, real* pdf, const Vec3& rand = Vec3(0, 0, 0)) const = 0;
	virtual Vec3 f(const Vec3& wo, const Vec3& wi) const = 0;
};

class DiffuseBSDF : public BxDF {
public:
	DiffuseBSDF(const Intersection &isect, Vec3 r) : BxDF(isect), R(r) {}

	//real Pdf(const Vec3 &wo, const Vec3 &wi) const override {
	//	return std::abs(wi.Dot(nl)) * INV_PI;
	//}

	real Pdf(const Vec3& wo, const Vec3& wi) const override {
		return SameHemisphere(wo, wi) ? BSDFCoordinate::AbsCosTheta(wi) * INV_PI : 0.f;
	}

	//Vec3 Sample_f(const Vec3 &wo, Vec3 *wi, real *pdf, const Vec3 &rand) const override {
	//	//real r1 = 2.f * PI * rand[0], r2 = rand[1];
	//	//real r2s = sqrt(r2);
	//	//Vec3 w = nl, u = ((fabs(w.x) > .1f ? Vec3(0, 1, 0) : Vec3(1, 0, 0)).Cross(w)).Norm();
	//	//Vec3 v = w.Cross(u);
	//	//*wi = (u* cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).Norm();


	//	Vec3 wiLocal = CosineSampleHemisphere(Vec2(rand[0], rand[1]));
	//	*wi = LocalToWorld(wiLocal);
	//	*pdf = Pdf(wo, *wi);
	//	return f(wo, *wi);
	//}

	Vec3 Sample_f(const Vec3& wo, Vec3* wi, real* pdf, const Vec3& rand) const override {

		*wi = CosineSampleHemisphere(Vec2(rand[0], rand[1]));
		*pdf = Pdf(wo, *wi);
		return f(wo, *wi);
	}

	Vec3 f(const Vec3 &wo, const Vec3 &wi) const override {
		return R * INV_PI;
	}
private:
	Vec3 R;
};

class SpecularBSDF : public BxDF {
public:
	/*SpecularBSDF(const Intersection &isect, Vec3 r = Vec3(1.0, 1.0, 1.0)) : BxDF(isect), R(r) {}*/

	SpecularBSDF(Vec3 r = Vec3(1.0, 1.0, 1.0)) : R(r) {}

	real Pdf(const Vec3 &wo, const Vec3 &wi) const override {
		return 0.0;
	}

	//Vec3 Sample_f(const Vec3 &wo, Vec3 *wi, real *pdf, const Vec3 &rand) const override {
	//	*wi = (nl * 2.0 * nl.Dot(wo) - wo).Norm();
	//	*pdf = 1.0;
	//	real cosTheta = std::abs((*wi).Dot(n));
	//	return R / cosTheta;
	//}

	Vec3 Sample_f(const Vec3& wo, Vec3* wi, real* pdf, const Vec3& rand) const override {
		*wi = Vec3(-wo.x, -wo.y, wo.z);
		*pdf = 1.0;
		real cosTheta = BSDFCoordinate::AbsCosTheta(*wi);
		return R / cosTheta;
	}

	Vec3 f(const Vec3 &wo, const Vec3 &wi) const override {
		return Vec3();
	}

	//bool IsDelta() const override { return true; }
private:
	Vec3 R;
};

//TODO

class TransmissionBSDF : public BSDF {
public:
	TransmissionBSDF(const Intersection &isect, Vec3 fa = Vec3(1.0, 1.0, 1.0), TransportMode mode = TransportMode::Radiance,
		real eta1 = 1.0, real eta2 = 1.5) :
		BSDF(isect), Fa(fa), nc(eta1), nt(eta2), transportMode(mode) {
	}

	real Pdf(const Vec3 &wo, const Vec3 &wi) const override {
		return 0.0;
	}

	Vec3 Sample_f(const Vec3 &wo, Vec3 *wi, real *pdf, const Vec3 &rand) const override {
		/*
		bool into = (n.Dot(nl) > 0.0);
		real nnt = into ? nc / nt : nt / nc, ddn = (-1 * wo).Dot(nl), cos2t;
		// total internal reflection
		if ((cos2t = 1 - nnt * nnt*(1 - ddn * ddn)) < 0) {
			*wi = (nl * 2.0 * nl.Dot(wo) - wo).Norm();
			real cosTheta = std::abs((*wi).Dot(n));
			*pdf = 1.0;
			return Fa / cosTheta;
			//std::cout << "total internal reflection" << std::endl;
			//return Vec3();
		}
		Vec3 td = ((-1 * wo) * nnt - n * ((into ? 1 : -1) * (ddn * nnt + sqrt(cos2t)))).Norm();
		real Re = Fresnell(wo, td, n, nl);*/


		real Re = FrDielectric(wo.Dot(n), nc, nt);

		bool entering = wo.Dot(n) > 0;
		real etaI = entering ? nc : nt;
		real etaT = entering ? nt : nc;
		Vec3 nForward = wo.Dot(n) > 0 ? n : -1 * n;
		real P = Re * 0.5f + 0.25f;
		//real P = Re;
		if (rand.z < P) {
			//{
			//	if (debugPixel == 1) {
			//		std::cout << "Reflection" << std::endl;
			//	}
			//}
			/*
			*wi = (nl * 2.0 * nl.Dot(wo) - wo).Norm();
			*pdf = P;
			real cosTheta = std::abs((*wi).Dot(n));
			return Fa * Re / cosTheta;
			*/
			*wi = (2 * wo.Dot(nForward) * nForward - wo).Norm();
			*pdf = P;
			real cosTheta = std::abs((*wi).Dot(nForward));
			return Fa * Re / cosTheta;
		}
		else {
			/*
			*wi = td;
			*pdf = 1 - P;
			real cosTheta = std::abs((*wi).Dot(n));
			return nnt * nnt * Fa * (1.0 - Re) / cosTheta;
			*/
			//{
			//	if (debugPixel == 1) {
			//		std::cout << "Refraction" << std::endl;
			//	}
			//}
			real cosThetaI = wo.Dot(nForward);
			real sinThetaI = std::sqrt(std::max(1 - cosThetaI * cosThetaI, (real)0));
			real sinThetaT = sinThetaI * etaI / etaT;
			if (sinThetaT >= 1.0) {
				*pdf = 1.0;
				*wi = (2 * wo.Dot(nForward) * nForward - wo).Norm();
				return Vec3();
			}
			real cosThetaT = std::sqrt(std::max(1 - sinThetaT * sinThetaT, (real)0));
			*wi = (cosThetaI * nForward - wo).Norm() * sinThetaT - nForward * cosThetaT;
			*pdf = 1 - P;
			real cosTheta = std::abs((*wi).Dot(nForward));
			real eta = etaI / etaT;
			if (transportMode == TransportMode::Radiance) {
				return eta * eta * Fa * (1.f - Re) / cosTheta;
			}
			else {
				return Fa * (1.f - Re) / cosTheta;
			}
		}
	}

	Vec3 f(const Vec3 &wo, const Vec3 &wi) const override {
		return Vec3();
	}

	real Fresnell(const Vec3 &wo, const Vec3 &td, const Vec3 &n, const Vec3 &nl) const {
		bool into = (n.Dot(nl) > 0.0);
		real nnt = into ? nc / nt : nt / nc, ddn = (-1 * wo).Dot(nl);//, cos2t;
		//cos2t = 1 - nnt * nnt*(1 - ddn * ddn);
		real a = nt - nc, b = nt + nc, R0 = a * a / (b * b), c = 1 - (into ? -ddn : td.Dot(n));
		real Re = R0 + (1 - R0) * c * c* c * c * c;
		return Re;
	}

	real FrDielectric(real cosThetaI, real etaI, real etaT) const {
		cosThetaI = Clamp(cosThetaI, -1.f, 1.f);
		// Potentially swap indices of refraction
		bool entering = cosThetaI > 0.f;
		if (!entering) {
			std::swap(etaI, etaT);
			cosThetaI = std::abs(cosThetaI);
		}

		// Compute _cosThetaT_ using Snell's law
		real sinThetaI = std::sqrt(std::max((real)0, 1 - cosThetaI * cosThetaI));
		real sinThetaT = etaI / etaT * sinThetaI;

		// Handle total internal reflection
		if (sinThetaT >= 1) return 1;
		real cosThetaT = std::sqrt(std::max((real)0, 1 - sinThetaT * sinThetaT));
		real Rparl = ((etaT * cosThetaI) - (etaI * cosThetaT)) /
			((etaT * cosThetaI) + (etaI * cosThetaT));
		real Rperp = ((etaI * cosThetaI) - (etaT * cosThetaT)) /
			((etaI * cosThetaI) + (etaT * cosThetaT));
		return (Rparl * Rparl + Rperp * Rperp) / 2;
	}

	bool IsDelta() const override { return true; }

	bool Refract(const Vec3 &wi, const Vec3 &n, real eta, Vec3 *wt) {
		// Compute $\cos \theta_\roman{t}$ using Snell's law
		real cosThetaI = n.Dot(wi);
		real sin2ThetaI = std::max(real(0), real(1 - cosThetaI * cosThetaI));
		real sin2ThetaT = eta * eta * sin2ThetaI;

		// Handle total internal reflection for transmission
		if (sin2ThetaT >= 1) return false;
		real cosThetaT = std::sqrt(1 - sin2ThetaT);
		*wt = eta * (-1 * wi) + (eta * cosThetaI - cosThetaT) * Vec3(n);
		return true;
	}
private:
	real nc, nt;
	Vec3 Fa;
	TransportMode transportMode;
};


class MicrofacetReflectionBSDF : public BSDF {
public:
	MicrofacetReflectionBSDF(const Intersection &isect, MicrofacetDistribution* distribution, const Vec3& R) :
		BSDF(isect), distribution(distribution), R(R) {

	}

	real Pdf(const Vec3& wo, const Vec3& wi) const override {
		Vec3 woLocal = WorldToLocal(wo);
		Vec3 wiLocal = WorldToLocal(wi);
		if (!SameHemisphere(woLocal, wiLocal)) return 0;
		Vector3f wh = (woLocal + wiLocal).Norm();
		return distribution->Pdf(woLocal, wh) / (4 * Dot(woLocal, wh));
	}

	Vec3 Sample_f(const Vec3& wo, Vec3* wi, real* pdf, const Vec3& rand) const override {
		Vec3 woLocal = WorldToLocal(wo);
		if (woLocal.z == 0) return 0.;
		Vector3f wh = distribution->Sample_wh(woLocal, Vec2(rand[0], rand[1]));
		Vec3 wiLocal = Reflect(woLocal, wh);
		if (!SameHemisphere(woLocal, wiLocal)) return Vec3(0, 0, 0);

		// Compute PDF of _wi_ for microfacet reflection
		*pdf = distribution->Pdf(woLocal, wh) / (4 * Dot(woLocal, wh));
		*wi = LocalToWorld(wiLocal);
		return f(woLocal, wiLocal);
	}

	Vec3 f(const Vec3& wo, const Vec3& wi) const override {
		Vec3 woLocal = WorldToLocal(wo);
		Vec3 wiLocal = WorldToLocal(wi);

		real cosThetaO = std::abs(CosTheta(woLocal)), cosThetaI = std::abs(CosTheta(wiLocal));
		Vec3 wh = wiLocal + woLocal;
		// Handle degenerate cases for microfacet reflection
		if (cosThetaI == 0 || cosThetaO == 0) return Vec3(0, 0, 0);
		if (wh.x == 0 && wh.y == 0 && wh.z == 0) return Vec3(0, 0, 0);
		Normalize(wh);
		//Spectrum F = fresnel->Evaluate(Dot(wi, wh));
		//return R * distribution->D(wh) * distribution->G(wo, wi) * F /
		//	(4 * cosThetaI * cosThetaO);
		return R * distribution->D(wh) * distribution->G(woLocal, wiLocal, wh) /
			(4 * cosThetaI * cosThetaO);
	}

	bool IsDelta() const override { return false; }
private:
	std::unique_ptr<MicrofacetDistribution> distribution;
	Vec3 R;
};

NAMESPACE_END