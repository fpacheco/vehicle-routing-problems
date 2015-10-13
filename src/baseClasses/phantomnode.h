#ifndef PHANTOMNODE_H
#define PHANTOMNODE_H

#include <iostream>

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
    PhantomNode(): mPoint( Point(0,0) ), mBeforePNode( Point(0,0) ), mAfterPNode( Point(0,0) )
    {}
    PhantomNode( UID phantomNodeId ): mPoint( Point(0,0) ), mBeforePNode( Point(0,0) ), mAfterPNode( Point(0,0) )
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

    void setName(std::string name) { mName = name; }
    std::string name () const { return mName; }

    void setNameId(UID nameId) { mNameId = nameId; }
    UID nameId () const { return mNameId; }

    void setBeforePNode (const Point &p) {
        mBeforePNode = Point( p.x(),p.y() );
    }
    Point beforePNode () const { return mBeforePNode; }

    void setAfterPNode (const Point &p) {
        mAfterPNode = Point( p.x(),p.y() );
    }
    Point afterPNode () const { return mAfterPNode; }

    void setBearing(double bearing) { mBearing = bearing; }
    UID bearing () const { return mBearing; }

    void setNetworkNNode (const Point &p) {
        mNetworkNNode = Point( p.x(),p.y() );
    }
    Point networkNNode () const { return mNetworkNNode; }

    bool inSameStreet(const PhantomNode &other);

    PhantomNode& operator= ( const PhantomNode &other );

private:
    UID mPhantomNodeId;     ///< internal node number
    Point mPoint;           ///< Longitude, Latitude
    UID mForwNodeId;        ///< OSRM forward node id
    UID mReveNodeId;        ///< OSRM reverse node id
    UID mForwWeight;        ///< OSRM forward weight
    UID mReveWeight;        ///< OSRM reverse weight
    std::string mName;      ///< OSRM street name
    UID mNameId;            ///< OSRM street name id
    Point mBeforePNode;     ///< Longitude and latitude of point before phantom node
    Point mAfterPNode;      ///< Longitude and latitude of point after phantom node
    double mBearing;        ///< Aproximate bearing to et container asocciated with this phantom node
    Point mNetworkNNode;    ///< Longitude and latitude of the nearest fisical node in the network
};

inline std::ostream &operator<<(std::ostream &out, const PhantomNode &pn)
{
    out << "id: " << pn.id() << ", "
        << "Point: " << pn.point() << ", "
        << "Forward node Id: " << pn.forwNodeId() << ", "
        << "Reverse node Id: " << pn.reveNodeId() << ", "
        << "Forward weight: " << pn.forwWeight() << ", "
        << "Reverse weight: " << pn.reveWeight() << ", "
        << "Name Id: " << pn.nameId() << ", "
        << "Before PN: " << pn.beforePNode() << ", "
        << "After PN: " << pn.afterPNode();
    return out;
}


#endif // PHANTOMNODE_H
