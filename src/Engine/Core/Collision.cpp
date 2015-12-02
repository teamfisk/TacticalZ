
namespace Collision
{

/*bool RayAABBIntr(float3 origin, float3 direction, float3 center, float3 h)
{
float3 w = 75.0f * direction;
float3 v = abs(w);
float3 c = origin - center + w;

//This is better if we hit often, do comparisons simultanously.
//if (any(abs(c) > v + h))
//	return false;
//
//return !(any(abs(c.yxx*w.zzy - c.zzy*w.yxx) > h.yxx*v.zzy + h.zzy*v.yxx));

//This is better if we miss often, reject test and exit early.
if (abs(c.x) > v.x + h.x) return false;
if (abs(c.y) > v.y + h.y) return false;
if (abs(c.z) > v.z + h.z) return false;

if (abs(c.y*w.z - c.z*w.y) > h.y*v.z + h.z*v.y) return false;
if (abs(c.x*w.z - c.z*w.x) > h.x*v.z + h.z*v.x) return false;
return !(abs(c.x*w.y - c.y*w.x) > h.x*v.y + h.y*v.x);
}*/

}