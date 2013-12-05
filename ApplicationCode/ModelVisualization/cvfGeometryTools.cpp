#include "cvfGeometryTools.h"

#pragma warning (disable : 4503)
namespace cvf
{


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d GeometryTools::computeFaceCenter(const cvf::Vec3d& v0, const cvf::Vec3d& v1, const cvf::Vec3d& v2, const cvf::Vec3d& v3)
{
    cvf::Vec3d centerCoord = v0;
    centerCoord += v1;
    centerCoord += v2;
    centerCoord += v3;
    centerCoord *= 0.25;

    return centerCoord;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

int GeometryTools::findClosestAxis(const cvf::Vec3d& vec )
{
    int closestAxis = 0;
    double maxComponent = fabs(vec.x());

    if (fabs(vec.y()) > maxComponent)
    {
        maxComponent = (float)fabs(vec.y());
        closestAxis = 1;
    }

    if (fabs(vec.z()) > maxComponent)
    {
        closestAxis = 2;
    }

    return closestAxis;
}

//--------------------------------------------------------------------------------------------------
/// Return angle between vectors if v1 x v2 is same way as normal
/// else return 2PI - angle
/// This means if the angle is slightly "negative", using the right hand rule, this method will return
/// nearly 2*PI
//--------------------------------------------------------------------------------------------------

double const MY_PI = 4 * atan(1.0);

double GeometryTools::getAngle(const cvf::Vec3d& positiveNormalAxis, const cvf::Vec3d& v1, const cvf::Vec3d& v2)
{
    bool isOk = false;
    cvf::Vec3d v1N = v1.getNormalized(&isOk);
    if (!isOk) return 0;
    cvf::Vec3d v2N = v2.getNormalized();
    if (!isOk) return 0;

    double cosAng = v1N * v2N;
    // Guard acos against out-of-domain input
    if (cosAng <= -1.0) 
    {
        cosAng = -1.0; 
    }
    else if (cosAng >= 1.0)
    {
        cosAng = 1.0; 
    }
    double angle = acos(cosAng);

    cvf::Vec3d crossProd = v1N ^ v2N;
    double sign = positiveNormalAxis * crossProd;
    if (sign < 0)
    {
        angle = 2*MY_PI - angle;
    }
    return angle;
}

//--------------------------------------------------------------------------------------------------
/// Return angle in radians between vectors [0, Pi]
/// If v1 or v2 is zero, the method will return 0.
//--------------------------------------------------------------------------------------------------

double GeometryTools::getAngle(const cvf::Vec3d& v1, const cvf::Vec3d& v2)
{
    bool isOk = false;
    cvf::Vec3d v1N = v1.getNormalized(&isOk);
    if (!isOk) return 0;
    cvf::Vec3d v2N = v2.getNormalized();
    if (!isOk) return 0;

    double cosAng = v1N * v2N;
    // Guard acos against out-of-domain input
    if (cosAng <= -1.0) 
    {
        cosAng = -1.0; 
    }
    else if (cosAng >= 1.0)
    {
        cosAng = 1.0; 
    }
    double angle = acos(cosAng);

    return angle;
}

/*
   Determine the intersection point of two line segments  
   From Paul Bourke, but modified to really handle coincident lines
   and lines with touching vertexes.
   Returns an intersection status telling what kind of intersection it is (if any)
   */

GeometryTools::IntersectionStatus inPlaneLineIntersect(
    double x1, double y1,
    double x2, double y2,
    double x3, double y3,
    double x4, double y4,
    double l1NormalizedTolerance, double l2NormalizedTolerance,
    double *x, double *y, double* fractionAlongLine1, double* fractionAlongLine2)
{
   double mua, mub;
   double denom, numera, numerb;

   denom  = (y4-y3) * (x2-x1) - (x4-x3) * (y2-y1);
   numera = (x4-x3) * (y1-y3) - (y4-y3) * (x1-x3);
   numerb = (x2-x1) * (y1-y3) - (y2-y1) * (x1-x3);

    double EPS = 1e-40;

   // Are the line coincident? 
   if (fabs(numera) < EPS && fabs(numerb) < EPS && fabs(denom) < EPS) 
   {
#if 0
       *x = 0;
       *y = 0;
       *fractionAlongLine1 = 0;
       *fractionAlongLine2 = 0;
       return GeometryTools::LINES_OVERLAP;
#else
       cvf::Vec3d p12(x2-x1, y2-y1, 0);
       cvf::Vec3d p13(x3-x1, y3-y1, 0);
       cvf::Vec3d p34(x4-x3, y4-y3, 0);

       double length12 = p12.length();
       double length34 = p34.length();

       // Check if the p1 p2 line is a point
      
       if (length12 < EPS )
       {
           cvf::Vec3d p34(x4-x3, y4-y3, 0);
           *x = x1;
           *y = y1;
           *fractionAlongLine1 = 1;
           *fractionAlongLine2 = p13.length()/p34.length();
           return GeometryTools::LINES_OVERLAP;
       }

       cvf::Vec3d p14(x4-x1, y4-y1, 0);
       cvf::Vec3d p32(x2-x3, y2-y3, 0);

       cvf::Vec3d e12 = p12.getNormalized(); 
       double normDist13 =  e12*p13 / length12;
       double normDist14 =  e12*p14 / length12;

       // Check if both points on the p3 p4 line is outside line p1 p2.
       if( (normDist13 < 0 - l1NormalizedTolerance && normDist14 < 0-l1NormalizedTolerance )|| (normDist13 > 1 +l1NormalizedTolerance  && normDist14 > 1+l1NormalizedTolerance ) ) 
      {
           *x = 0;
           *y = 0;
           *fractionAlongLine1 = 0;
           *fractionAlongLine2 = 0;
          return GeometryTools::NO_INTERSECTION;
       }

       double normDist32 =  e12*p32 / length34;
       double normDist31 = -e12*p13 / length34;

       // Set up fractions along lines to the edge2 vertex actually touching edge 1.
       /// if two, select the one furthest from the start
       bool pt3IsInside = false;
       bool pt4IsInside = false;
       if ((0.0 - l1NormalizedTolerance) <= normDist13 && normDist13 <= (1.0 +l1NormalizedTolerance) ) pt3IsInside = true;
       if ((0.0 - l1NormalizedTolerance) <= normDist14 && normDist14 <= (1.0 +l1NormalizedTolerance) ) pt4IsInside = true;

       if (pt3IsInside && !pt4IsInside)
       {
           *fractionAlongLine1 = normDist13;
           *fractionAlongLine2 = 0.0;
           *x = x3;
           *y = y3;
       }
       else if (pt4IsInside && !pt3IsInside)
       {
           *fractionAlongLine1 = normDist14;
           *fractionAlongLine2 = 1.0;
           *x = x4;
           *y = y4;
       }
       else if (pt3IsInside && pt4IsInside)
       {
           // Return edge 2 vertex furthest along edge 1
           if (normDist13 <= normDist14)
           {
               *fractionAlongLine1 =  normDist14 ;
               *fractionAlongLine2 =  1.0;
               *x = x4;
               *y = y4;
           }
           else
           {
               *fractionAlongLine1 = normDist13;
               *fractionAlongLine2 =  0.0;
               *x = x3;
               *y = y3;
           }
      }
       else // both outside on each side
       {
           // Return End of edge 1
            *fractionAlongLine1 = 1.0;
            *fractionAlongLine2 = normDist32;
            *x = x2;
            *y = y2;
       } 
  
      return GeometryTools::LINES_OVERLAP;
#endif
   }

   /* Are the line parallel */
   if (fabs(denom) < EPS) {
      *x = 0;
      *y = 0;
      *fractionAlongLine1 = 0;
      *fractionAlongLine2 = 0;

      return GeometryTools::NO_INTERSECTION;
   }

   /* Is the intersection along the the segments */
   mua = numera / denom;
   mub = numerb / denom;

   *x = x1 + mua * (x2 - x1);
   *y = y1 + mua * (y2 - y1);
   *fractionAlongLine1 = mua;
   *fractionAlongLine2 = mub;

   if (mua < 0 - l1NormalizedTolerance || 1 + l1NormalizedTolerance < mua  || mub < 0 - l2NormalizedTolerance ||  1 + l2NormalizedTolerance < mub) 
   {
      return GeometryTools::LINES_INTERSECT_OUTSIDE;
   }
   else if (fabs(mua) < l1NormalizedTolerance || fabs(1-mua) < l1NormalizedTolerance || 
            fabs(mub) < l2NormalizedTolerance || fabs(1-mub) < l2NormalizedTolerance )
   {
       if (fabs(mua)   < l1NormalizedTolerance) *fractionAlongLine1 = 0;
       if (fabs(1-mua) < l1NormalizedTolerance) *fractionAlongLine1 = 1;
       if (fabs(mub)   < l2NormalizedTolerance) *fractionAlongLine2 = 0;
       if (fabs(1-mub) < l2NormalizedTolerance) *fractionAlongLine2 = 1;
       return GeometryTools::LINES_TOUCH;
   }
   else
   {
       return GeometryTools::LINES_CROSSES;
   }
}
//----------------------------------------------------------------------------------------------------------
/// Supposed to find the intersection point if lines intersect
/// It returns the intersection status telling if the lines only touch or are overlapping
//----------------------------------------------------------------------------------------------------------

GeometryTools::IntersectionStatus 
GeometryTools::inPlaneLineIntersect3D(  const cvf::Vec3d& planeNormal, 
                                        const cvf::Vec3d& p1, const cvf::Vec3d& p2, const cvf::Vec3d& p3, const cvf::Vec3d& p4, 
                                        cvf::Vec3d* intersectionPoint, double* fractionAlongLine1, double* fractionAlongLine2,  double tolerance)
{
    CVF_ASSERT (intersectionPoint != NULL);

    int Z = findClosestAxis(planeNormal);
    int X = (Z + 1) % 3;
    int Y = (Z + 2) % 3;
    double x, y;

    // Todo: handle zero length edges
    double l1NormTol = tolerance / (p2-p1).length();
    double l2NormTol = tolerance / (p4-p3).length();

    IntersectionStatus intersectionStatus = inPlaneLineIntersect(p1[X], p1[Y], p2[X], p2[Y], p3[X], p3[Y], p4[X], p4[Y], l1NormTol, l2NormTol, &x, &y, fractionAlongLine1, fractionAlongLine2);

    // Check if we have a valid intersection point
    if (intersectionStatus == NO_INTERSECTION || intersectionStatus == LINES_OVERLAP)
    {
        intersectionPoint->setZero();
    }
    else
    {
        *intersectionPoint = p1 + (*fractionAlongLine1)*(p2-p1);
    }
   
    return intersectionStatus;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double	GeometryTools::linePointSquareDist(const cvf::Vec3d& p1, const cvf::Vec3d& p2, const cvf::Vec3d& p3)
{
    cvf::Vec3d v31 = p3 - p1;
    cvf::Vec3d v21 = p2 - p1;

    double geomTolerance = 1e-24;
    if (v21.lengthSquared() < geomTolerance)
    {
        // P2 and P1 coincide, use distance from P3 to P1
        return v31.lengthSquared();
    }

    double u = (v31*v21)/(v21*v21);
    cvf::Vec3d pOnLine(0,0,0);
    if (0 < u && u < 1) pOnLine = p1 + u*v21;
    else if (u <= 0 ) pOnLine = p1;
    else pOnLine = p2;

    return (p3-pOnLine).lengthSquared();
}

//--------------------------------------------------------------------------------------------------
// Copyright 2001, softSurfer (www.softsurfer.com)
// This code may be freely used and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.
// http://www.softsurfer.com/Archive/algorithm_0105/algorithm_0105.htm
//
/// Intersect a line segment with a 3D triangle
///    Input:  A line segment p0, p1. A triangle t0, t1, t2.
///    Output: *intersectionPoint = intersection point (when it exists)
///    Return: -1 = triangle is degenerate (a segment or point)
///             0 = disjoint (no intersect)
///             1 = intersect in unique point I1
///             2 = are in the same plane
//--------------------------------------------------------------------------------------------------

#define SMALL_NUM  0.00000001 // anything that avoids division overflow
// dot product (3D) which allows vector operations in arguments
#define dot(u,v)   ((u).x() * (v).x() + (u).y() * (v).y() + (u).z() * (v).z())

int GeometryTools::intersectLineSegmentTriangle( const cvf::Vec3d p0, const cvf::Vec3d p1, 
                                                 const cvf::Vec3d t0, const cvf::Vec3d t1, const cvf::Vec3d t2,
                                                 cvf::Vec3d* intersectionPoint )
{
    CVF_ASSERT(intersectionPoint != NULL);
    cvf::Vec3d u, v, n;             // triangle vectors
    cvf::Vec3d dir, w0, w;          // ray vectors
    double     r, a, b;             // params to calc ray-plane intersect

    // get triangle edge vectors and plane normal
    u = t1 - t0;
    v = t2 - t0;
    n = u ^ v;                     // cross product
    if (n == cvf::Vec3d::ZERO)     // triangle is degenerate
        return -1;                 // do not deal with this case

    dir = p1 - p0;                 // ray direction vector
    w0  = p0 - t0;
    a   = -dot(n, w0);
    b   =  dot(n, dir);
    if (fabs(b) < SMALL_NUM) {     // ray is parallel to triangle plane
        if (a == 0)                // ray lies in triangle plane
            return 2;
        else return 0;             // ray disjoint from plane
    }

    // get intersect point of ray with triangle plane
    r = a / b;
    if (r < 0.0)                   // ray goes away from triangle
        return 0;                  // => no intersect

    if (r > 1.0)                   // Line segment does not reach triangle
        return 0;

    *intersectionPoint = p0 + r * dir;             // intersect point of ray and plane

    // is I inside T?
    double    uu, uv, vv, wu, wv, D;
    uu = dot(u, u);
    uv = dot(u, v);
    vv = dot(v, v);
    w = *intersectionPoint - t0;
    wu = dot(w, u);
    wv = dot(w, v);
    D = uv * uv - uu * vv;

    // get and test parametric coords
    double s, t;
    s = (uv * wv - vv * wu) / D;
    if (s < 0.0 || s > 1.0)        // I is outside T
        return 0;

    t = (uv * wu - uu * wv) / D;
    if (t < 0.0 || (s + t) > 1.0)  // I is outside T
        return 0;

    return 1;                      // I is in T
}

/*
//    t0 = (x0, y0, z0)
//    t1 = (x1, y1, z1)
//    t2 = (x2, y2, z2)
//
//    p = (xp, yp, zp)

cvf::Vec3d barycentricCoordsExperiment(const cvf::Vec3d& t0, const cvf::Vec3d& t1, const cvf::Vec3d& t2, const cvf::Vec3d& p)
{
    det = x0(y1*z2 - y2*z1) + x1(y2*z0 - z2*y0) + x2(y0*z1 - y1*z0);

    b0 = ((x1 * y2 - x2*y1)*zp + xp*(y1*z2-y2*z1) + yp*(x2*z1-x1*z2)) / det;
    b1 = ((x2 * y0 - x0*y2)*zp + xp*(y2*z0-y0*z2) + yp*(x0*z2-x2*z0)) / det;
    b2 = ((x0 * y1 - x1*y0)*zp + xp*(y0*z1-y1*z0) + yp*(x1*z0-x0*z1)) / det;
}

*/

inline double TriArea2D(double x1, double y1, double x2, double y2, double x3, double y3)
{
    return (x1-x2)*(y2-y3) - (x2-x3)*(y1-y2);
}

//--------------------------------------------------------------------------------------------------
// Compute barycentric coordinates (area coordinates) (u, v, w) for 
// point p with respect to triangle (t0, t1, t2)
// These can be used as weights for interpolating scalar values across the triangle
// Based on section 3.4 in "Real Time collision detection" by Christer Ericson
//--------------------------------------------------------------------------------------------------
cvf::Vec3d GeometryTools::barycentricCoords(const cvf::Vec3d&  t0, const cvf::Vec3d&  t1, const cvf::Vec3d&  t2, const cvf::Vec3d&  p)
{
    // Unnormalized triangle normal
    cvf::Vec3d m = (t1 - t0 ^ t2 - t0);

    // Absolute components for determining projection plane
    int X = 0, Y = 1, Z = 2;
    Z = findClosestAxis(m);
    switch (Z)
    {
    case 0: X = 1; Y = 2; break; // x is largest, project to the yz plane
    case 1: X = 0; Y = 2; break; // y is largest, project to the xz plane
    case 2: X = 0; Y = 1; break; // z is largest, project to the xy plane
    }

    // Compute areas in plane of largest projection
    // Nominators and one-over-denominator for u and v ratios
    double nu, nv, ood;
    nu = TriArea2D(p[X], p[Y], t1[X], t1[Y], t2[X], t2[Y]); // Area of PBC in yz plane
    nv = TriArea2D(p[X], p[Y], t2[X], t2[Y], t0[X], t0[Y]); // Area of PCA in yz plane
    ood = 1.0f / m[Z];                             // 1/(2*area of ABC in yz plane)

    if (Z == 1) ood = -ood; // For some reason not explained

    // Normalize

    m[0] = nu * ood;
    m[1] = nv * ood;
    m[2] = 1.0f - m[0] - m[1];

    return m;
}

//--------------------------------------------------------------------------------------------------
/// Inserts the vertex into the polygon if it fits along one of the edges within the tolerance.
/// The method returns true if it was inserted, or if it was already in the polygon, or if it was 
/// within the tolerance of an existing vertex in the polygon. 
/// In the latter situation it replaces the previous vertex in the polygon.
/// 
/// Todo: If a vertex is replaced, the VxToCv map in TimeStepGeometry should be updated
//--------------------------------------------------------------------------------------------------
bool GeometryTools::insertVertexInPolygon(std::list<std::pair<cvf::uint, bool> >* polygon, const cvf::Vec3dArray& nodeCoords, cvf::uint vertexIndex, double tolerance)
{
    std::list<std::pair<cvf::uint, bool> >::iterator it;
    for(it = polygon->begin(); it != polygon->end(); ++it)
    {
        if (it->first == vertexIndex) return true;
    }


#if 1
    bool existsOrInserted = false;
    for(it = polygon->begin(); it != polygon->end(); ++it)
    {
        if ( (nodeCoords[it->first] - nodeCoords[vertexIndex]).length() < tolerance)
        {
            if (vertexIndex < it->first) it->first = vertexIndex;
            existsOrInserted = true;
        }
    }

    if (existsOrInserted) return true;
#endif

    // Insert vertex in polygon if the distance to one of the edges is small enough

    std::list<std::pair<cvf::uint, bool> >::iterator it2;
    std::list<std::pair<cvf::uint, bool> >::iterator insertBefore;

    for (it = polygon->begin(); it != polygon->end(); ++it)
    {
        it2 = it;
        ++it2; insertBefore = it2; if (it2 == polygon->end()) it2 = polygon->begin();

        double sqDistToLine = GeometryTools::linePointSquareDist(nodeCoords[it->first], nodeCoords[it2->first], nodeCoords[vertexIndex]);
        if (fabs(sqDistToLine) < tolerance*tolerance )
        {
            it = polygon->insert(insertBefore, std::make_pair(vertexIndex, false));
            existsOrInserted = true;
        }
    }

    return existsOrInserted;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void GeometryTools::addMidEdgeNodes(std::list<std::pair<cvf::uint, bool> >* polygon, const cvf::Vec3dArray& nodes, EdgeSplitStorage& edgeSplitStorage, std::vector<cvf::Vec3d>* createdVertexes)
{
    size_t newVertexIndex = nodes.size() + createdVertexes->size();
    std::list<std::pair<cvf::uint, bool> >::iterator it;
    std::list<std::pair<cvf::uint, bool> >::iterator it2;

    cvf::Vec3d midEdgeCoord(0,0,0);
    size_t midPointIndex = cvf::UNDEFINED_UINT;

    for (it = polygon->begin(); it != polygon->end(); ++it)
    {
        it2 = it;
        ++it2; if (it2 == polygon->end()) it2 = polygon->begin();

        // Find or Create and add a mid-edge node

        if (!edgeSplitStorage.findSplitPoint(it->first, it2->first, &midPointIndex))
        {

            midEdgeCoord.setZero();
            midEdgeCoord += (it->first  < nodes.size()) ? nodes[it->first]  : (*createdVertexes)[it->first - nodes.size()];
            midEdgeCoord += (it2->first < nodes.size()) ? nodes[it2->first] : (*createdVertexes)[it2->first - nodes.size()];
            midEdgeCoord *= 0.5;

            midPointIndex = newVertexIndex;
            createdVertexes->push_back(midEdgeCoord);
            ++newVertexIndex;

            edgeSplitStorage.addSplitPoint(it->first, it2->first, midPointIndex);
        }

        if (it2 != polygon->begin())
            polygon->insert(it2, std::make_pair((cvf::uint)midPointIndex, true));
        else 
            polygon->insert(polygon->end(), std::make_pair((cvf::uint)midPointIndex, true));

        ++it;
    }
}




//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void EdgeSplitStorage::setVertexCount(size_t size)
{
     m_edgeSplitMap.resize(size);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool EdgeSplitStorage::findSplitPoint(size_t edgeP1Index, size_t edgeP2Index, size_t* splitPointIndex)
{
    canonizeAddress(edgeP1Index, edgeP2Index);
    CVF_ASSERT(edgeP1Index < m_edgeSplitMap.size());    

    std::map< size_t, size_t >::iterator it;

    it = m_edgeSplitMap[edgeP1Index].find(edgeP2Index);
    if (it == m_edgeSplitMap[edgeP1Index].end()) return false;

    *splitPointIndex = it->second;
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void EdgeSplitStorage::addSplitPoint(size_t edgeP1Index, size_t edgeP2Index, size_t splitPointIndex)
{
    canonizeAddress(edgeP1Index, edgeP2Index);
    CVF_ASSERT(edgeP1Index < m_edgeSplitMap.size());    
    m_edgeSplitMap[edgeP1Index][edgeP2Index] = splitPointIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void EdgeSplitStorage::canonizeAddress(size_t& edgeP1Index, size_t& edgeP2Index)
{
    if (edgeP1Index > edgeP2Index)
    {
        size_t tmp;
        tmp = edgeP1Index;
        edgeP1Index = edgeP2Index;
        edgeP2Index = tmp;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
EarClipTesselator::EarClipTesselator(): 
    m_X(-1), 
    m_Y(-1), 
    m_areaTolerance(1e-12), 
    m_nodeCoords(NULL)
{

}

//--------------------------------------------------------------------------------------------------
/// \brief  	Do the main processing/actual triangulation
/// \param  	triangleIndices Array that will receive the indices of the triangles resulting from the triangulation
/// \return	    true when a tesselation was successully created 
//--------------------------------------------------------------------------------------------------

bool EarClipTesselator::calculateTriangles( std::vector<size_t>* triangleIndices ) 
{
    CVF_ASSERT(m_nodeCoords != NULL);
    CVF_ASSERT(m_X > -1 && m_Y > -1);

    size_t numVertices = m_polygonIndices.size();

    if (numVertices < 3) return false;

    // We want m_polygonIndices to be a counter-clockwise polygon to make the validation test work

	if (calculatePolygonArea() < 0 )
	{
		m_polygonIndices.reverse();
	}

    std::list<size_t>::iterator u, v, w;

    // If we loop two times around polygon without clipping a single triangle we are toast.
	size_t count = 2*numVertices;   // error detection 

	v = m_polygonIndices.end();   //nv - 1;
    --v;

	while (numVertices > 2)
	{
		// if we loop, it is probably a non-simple polygon 
		if (count <= 0 )
		{
			// Triangulate: ERROR - probable bad polygon!
			return false;
		}
        --count; 

		// Three consecutive vertices in current polygon, <u,v,w> 
		// previous 
	    u = v; 
		if (u == m_polygonIndices.end()) u =  m_polygonIndices.begin(); // if (nv <= u) u = 0;     

		// new v
		v = u; ++v; //u + 1; 
		if (v == m_polygonIndices.end()) v =  m_polygonIndices.begin(); //if (nv <= v) v = 0;

		// next
		w = v; ++w; //v + 1; 
		if (w == m_polygonIndices.end()) w =  m_polygonIndices.begin(); //if (nv <= w) w = 0;     


		if ( isTriangleValid(u, v, w) )
		{
			// Indices of the vertices 
			triangleIndices->push_back(*u);
			triangleIndices->push_back(*v);
			triangleIndices->push_back(*w);

			// Remove v from remaining polygon 
            m_polygonIndices.erase(v);
            v = w;
			numVertices--;

			// Resets error detection counter 
			count = 2*numVertices;
		}
	}

	return true;
}


//--------------------------------------------------------------------------------------------------
/// Is this a valid triangle ? ( No points inside, and points not on a line. )  
//--------------------------------------------------------------------------------------------------

bool EarClipTesselator::isTriangleValid( std::list<size_t>::const_iterator u, std::list<size_t>::const_iterator v, std::list<size_t>::const_iterator w) const
{
    CVF_ASSERT(m_X > -1 && m_Y > -1);

    cvf::Vec3d A = (*m_nodeCoords)[*u];
    cvf::Vec3d B = (*m_nodeCoords)[*v];
    cvf::Vec3d C = (*m_nodeCoords)[*w];

    if (  m_areaTolerance > (((B[m_X]-A[m_X])*(C[m_Y]-A[m_Y])) - ((B[m_Y]-A[m_Y])*(C[m_X]-A[m_X]))) ) return false;

	std::list<size_t>::const_iterator c;
    std::list<size_t>::const_iterator outside;
	for (c = m_polygonIndices.begin(); c != m_polygonIndices.end(); ++c)
	{
        // The polygon points that actually make up the triangle candidate does not count
        // (but the same points on different positions in the polygon does! 
        // Except those one off the triangle, that references the start or end of the triangle)

		if ( (c == u) || (c == v) || (c == w)) continue;

        // Originally the below tests was not included which resulted in missing triangles sometimes

        outside = w; ++outside; if (outside == m_polygonIndices.end()) outside = m_polygonIndices.begin();
        if (c == outside && *c == *u) 
        {
            continue;
        }

        outside = u; if (outside == m_polygonIndices.begin()) outside = m_polygonIndices.end(); --outside; 
        if (c == outside && *c == *w) 
        {
            continue;
        }

        cvf::Vec3d P = (*m_nodeCoords)[*c];

		if (isPointInsideTriangle(A, B, C, P)) return false;
	}

	return true;
}


//--------------------------------------------------------------------------------------------------
/// Decides if a point P is inside of the triangle defined by A, B, C.
/// By calculating the "double area" (cross product) of Corner to corner x Corner to point vectors
//--------------------------------------------------------------------------------------------------

bool EarClipTesselator::isPointInsideTriangle(const cvf::Vec3d& A, const cvf::Vec3d& B, const cvf::Vec3d& C, const cvf::Vec3d& P) const
{
    CVF_ASSERT(m_X > -1 && m_Y > -1);
    
	double ax = C[m_X] - B[m_X];  double ay = C[m_Y] - B[m_Y];
	double bx = A[m_X] - C[m_X];  double by = A[m_Y] - C[m_Y];
	double cx = B[m_X] - A[m_X];  double cy = B[m_Y] - A[m_Y];

	double apx= P[m_X] - A[m_X];  double apy= P[m_Y] - A[m_Y];
	double bpx= P[m_X] - B[m_X];  double bpy= P[m_Y] - B[m_Y];
	double cpx= P[m_X] - C[m_X];  double cpy= P[m_Y] - C[m_Y];

	double aCROSSbp = ax*bpy - ay*bpx;
	double cCROSSap = cx*apy - cy*apx;
	double bCROSScp = bx*cpy - by*cpx;
    double tol = 0;
	return ((aCROSSbp >= tol) && (bCROSScp >= tol) && (cCROSSap >= tol));
};

//--------------------------------------------------------------------------------------------------
/// Computes area of the currently stored 2D polygon/contour
//--------------------------------------------------------------------------------------------------

double EarClipTesselator::calculatePolygonArea() const
{
    CVF_ASSERT(m_X > -1 && m_Y > -1);

	double A = 0;

	std::list<size_t>::const_iterator p = m_polygonIndices.end();
    --p;

	std::list<size_t>::const_iterator q = m_polygonIndices.begin();
	while (q != m_polygonIndices.end())
	{
		A += (*m_nodeCoords)[*p][m_X] * (*m_nodeCoords)[*q][m_Y] - (*m_nodeCoords)[*q][m_X]*(*m_nodeCoords)[*p][m_Y];

		p = q;
		q++;
	}

	return A*0.5;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void EarClipTesselator::setNormal(const cvf::Vec3d& polygonNormal)
{
    int Z = GeometryTools::findClosestAxis(polygonNormal);
     m_X = (Z + 1) % 3;
     m_Y = (Z + 2) % 3;
     m_polygonNormal = polygonNormal;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void EarClipTesselator::setPolygonIndices(const std::list<size_t>& polygon)
{
    m_polygonIndices = polygon;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void EarClipTesselator::setPolygonIndices(const std::vector<size_t>& polygon)
{
    size_t i;
    for (i = 0; i < polygon.size();  ++i)
    {
        m_polygonIndices.push_back(polygon[i]);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void EarClipTesselator::setMinTriangleArea(double areaTolerance)
{
    m_areaTolerance = 2*areaTolerance; // Convert to trapesoidal area
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void EarClipTesselator::setGlobalNodeArray(const cvf::Vec3dArray& nodeCoords)
{
    m_nodeCoords = &nodeCoords;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
FanEarClipTesselator::FanEarClipTesselator() : 
    m_centerNodeIndex(std::numeric_limits<size_t>::max())
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FanEarClipTesselator::setCenterNode(size_t centerNodeIndex)
{
    m_centerNodeIndex = centerNodeIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool FanEarClipTesselator::calculateTriangles(std::vector<size_t>* triangles)
{
    CVF_ASSERT(m_centerNodeIndex != std::numeric_limits<size_t>::max());
    CVF_ASSERT(m_nodeCoords != NULL);
    CVF_ASSERT(m_X > -1 && m_Y > -1);

    size_t nv = m_polygonIndices.size();

    if (nv < 3) return false;

    // We want m_polygonIndices to be a counter-clockwise polygon to make the validation test work

    if (calculatePolygonArea() < 0 )
    {
        m_polygonIndices.reverse();
    }

    std::list<size_t>::const_iterator it1;
    std::list<size_t>::const_iterator it2;

    std::list< std::list<size_t> > restPolygons;
    bool wasPreviousTriangleValid = true;

    for (it1 = m_polygonIndices.begin(); it1 != m_polygonIndices.end(); it1++)
    {
        it2 = it1;
        it2++;

        if (it2 == m_polygonIndices.end()) it2 = m_polygonIndices.begin();

        if (isTriangleValid(*it1, *it2, m_centerNodeIndex))
        {
            triangles->push_back(*it1);
            triangles->push_back(*it2);
            triangles->push_back(m_centerNodeIndex);
            wasPreviousTriangleValid = true;
        }
        else
        {
            if (wasPreviousTriangleValid)
            {
                // Create new rest polygon.
                restPolygons.push_back(std::list<size_t>());
                restPolygons.back().push_back(m_centerNodeIndex);
                restPolygons.back().push_back(*it1);
                restPolygons.back().push_back(*it2);
            }
            else
            {
                restPolygons.back().push_back(*it2);
            }
        }
    }

    EarClipTesselator triMaker;
    triMaker.setNormal(m_polygonNormal);
    triMaker.setMinTriangleArea(m_areaTolerance);
    triMaker.setGlobalNodeArray(*m_nodeCoords);
    std::list< std::list<size_t> >::iterator rpIt;

    for (rpIt = restPolygons.begin(); rpIt != restPolygons.end(); ++rpIt)
    {
        triMaker.setPolygonIndices(*rpIt);
        triMaker.calculateTriangles(triangles);
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// This needs to be rewritten because we need to test for crossing edges, not only point inside.
/// In addition the test for polygon 
//--------------------------------------------------------------------------------------------------
bool FanEarClipTesselator::isTriangleValid(size_t u, size_t v, size_t w)
{
    CVF_ASSERT(m_X > -1 && m_Y > -1);

    cvf::Vec3d A = (*m_nodeCoords)[u];
    cvf::Vec3d B = (*m_nodeCoords)[v];
    cvf::Vec3d C = (*m_nodeCoords)[w];

    if (  m_areaTolerance > (((B[m_X]-A[m_X])*(C[m_Y]-A[m_Y])) - ((B[m_Y]-A[m_Y])*(C[m_X]-A[m_X]))) ) return false;

    std::list<size_t>::const_iterator c;
    for (c = m_polygonIndices.begin(); c != m_polygonIndices.end(); ++c)
    {
        // The polygon points that actually make up the triangle candidate does not count
        // (but the same points on different positions in the polygon does! )
        // Todo so this test below is to accepting !! Bug !!
        if ( (*c == u) || (*c == v) || (*c == w)) continue;

        cvf::Vec3d P = (*m_nodeCoords)[*c];

        if (isPointInsideTriangle(A, B, C, P)) return false;
    }

    return true;
}


}