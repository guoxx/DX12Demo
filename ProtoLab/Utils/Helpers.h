#pragma once

// https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
inline void EulerianToEulerAngle(const DirectX::FXMVECTOR q, double& pitch, double& yaw, double& roll)
{
	float qx = DirectX::XMVectorGetX(q);
	float qy = DirectX::XMVectorGetY(q);
	float qz = DirectX::XMVectorGetZ(q);
	float qw = DirectX::XMVectorGetW(q);

	double ysqr = qy * qy;
	double t0 = -2.0f * (ysqr + qz * qz) + 1.0f;
	double t1 = +2.0f * (qx * qy + qw * qz);
	double t2 = -2.0f * (qx * qz - qw * qy);
	double t3 = +2.0f * (qy * qz + qw * qx);
	double t4 = -2.0f * (qx * qx + ysqr) + 1.0f;

	t2 = t2 > 1.0f ? 1.0f : t2;
	t2 = t2 < -1.0f ? -1.0f : t2;

	pitch = std::asin(t2);
	yaw = std::atan2(t1, t0);
	roll = std::atan2(t3, t4);
}