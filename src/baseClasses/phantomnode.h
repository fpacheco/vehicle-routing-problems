#ifndef PHANTOMNODE_H
#define PHANTOMNODE_H

#include <iostream>
#include <string>

#include <basictypes.h>

class Point
{
public:
  Point(): mX(0),mY(0) {
  }
  Point(const Point &other) {
      mX = other.x();
      mY = other.y();
  }
  Point(double x, double y) {
      mX = x;
      mY = y;
  }
  double x () const { return mX; }
  double y () const { return mY; }

  Point& operator= ( const Point &other ){
    mX = other.x();
    mY = other.y();
    return *this;
  }

private:
  double mX;
  double mY;
};

inline std::ostream &operator<<(std::ostream &out, const Point &p)
{
    out << " (" << p.x() << ", " << p.y() << ") ";
    return out;
}


/*! \class PhantomNode
 * \brief This class implement information of Phantom Nodes
 *
 * OSRM phantom nodes are virtual nodes. Nearest point in an edge from input point
 *
 */
class PhantomNode
{
public:
    PhantomNode(): mBearing(-1.0), mStreetName( std::string() ), mPoint( Point(0,0) )
    {}
    PhantomNode( UID phantomNodeId ): mBearing(-1.0), mStreetName( std::string() ), mPoint( Point(0,0) )
    { mPhantomNodeId = phantomNodeId; }
    PhantomNode( UID phantomNodeId, double x, double y, UID fwNodeId, UID rvNodeId, UID fwWeight, UID rvWeight, UID nameId );
    PhantomNode( const PhantomNode &other );

    void setId(UID phantomNodeId) { mPhantomNodeId = phantomNodeId; }
    UID id () const { return mPhantomNodeId; }

    void setPoint (const Point &p) {
        mPoint = Point(p.x(),p.y());
    }
    Point point () const { return mPoint; }

    void setForwNodeId(UID fwNodeId) { mForwNodeId = fwNodeId; }
    UID forwNodeId () const { return mForwNodeId; }

    void setReveNodeId(UID rvNodeId) { mReveNodeId = rvNodeId; }
    UID reveNodeId () const { return mReveNodeId; }

    void setForwWeight(UID fwWeight) { mForwWeight = fwWeight; }
    UID forwWeight () const { return mForwWeight; }

    void setReveWeight(UID rvWeight) { mReveWeight = rvWeight; }
    UID reveWeight () const { return mReveWeight; }

    void setStreetName(std::string name) {
      mStreetName = name;
    }
    std::string streetName () const { return mStreetName; }

    void setNameId(UID nameId) { mNameId = nameId; }
    UID nameId () const { return mNameId; }

    void setBearing(double bearing) { mBearing = bearing; }
    double bearing () const { return mBearing; }

    bool inSameStreet(const PhantomNode &other);

    PhantomNode& operator= ( const PhantomNode &other );

private:
    UID mPhantomNodeId;     ///< internal node number
    Point mPoint;           ///< Longitude, Latitude
    UID mForwNodeId;        ///< OSRM forward node id
    UID mReveNodeId;        ///< OSRM reverse node id
    UID mForwWeight;        ///< OSRM forward weight
    UID mReveWeight;        ///< OSRM reverse weight
    std::string mStreetName; ///< OSRM street name
    UID mNameId;            ///< OSRM street name id
    double mBearing;        ///< Aproximate bearing of the container asocciated with this phantom node
};

inline std::ostream &operator<<(std::ostream &out, const PhantomNode &pn)
{
    out << "id: " << pn.id() << ", "
        << "Point: " << pn.point() << ", "
        << "Forward node Id: " << pn.forwNodeId() << ", "
        << "Reverse node Id: " << pn.reveNodeId() << ", "
        << "Forward weight: " << pn.forwWeight() << ", "
        << "Reverse weight: " << pn.reveWeight() << ", "
        << "Bearing: " << pn.bearing() << ", "
        // << "Name: " << pn.streetName() << ", "
        << "Name Id: " << pn.nameId();
    return out;
}


#endif // PHANTOMNODE_H
