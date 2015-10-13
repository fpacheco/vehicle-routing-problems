#include "phantomnode.h"

PhantomNode::PhantomNode(UID phantomNodeId, double x, double y, UID fwNodeId, UID rvNodeId, UID fwWeight, UID rvWeight, UID nameId):
    mBearing(-1.0), mBeforePNode( Point(0,0) ), mAfterPNode( Point(0,0) ), mNetworkNNode(0,0)
{
    mPhantomNodeId = phantomNodeId;
    mPoint = Point(x, y);
    mForwNodeId = fwNodeId;
    mReveNodeId = rvNodeId;
    mForwWeight = fwWeight;
    mReveWeight = rvWeight;
    mNameId = nameId;
}

PhantomNode::PhantomNode(const PhantomNode &other) {
    mPhantomNodeId = other.mPhantomNodeId;
    mPoint = other.mPoint;
    mForwNodeId = other.mForwNodeId;
    mReveNodeId = other.mReveNodeId;
    mForwWeight = other.mForwWeight;
    mReveWeight = other.mReveWeight;
    mBeforePNode = other.mBeforePNode;
    mAfterPNode = other.mAfterPNode;
    mNetworkNNode = other.mNetworkNNode;
    mBearing = other.mBearing;
    // setStreetName( other.streetName() );
    // mStreetName.assign(other.mStreetName);
    mNameId = other.mNameId;
}

bool PhantomNode::inSameStreet(const PhantomNode &other)
{
    return (
        mNameId == other.mNameId &&
            (
                (mForwNodeId == other.mForwNodeId && mReveNodeId == other.mReveNodeId) ||
                (mForwNodeId == other.mReveNodeId && mReveNodeId == other.mForwNodeId)
            )
    );
}

PhantomNode& PhantomNode::operator= (const PhantomNode &other)
{
    mPhantomNodeId = other.mPhantomNodeId;
    mPoint = other.mPoint;
    mForwNodeId = other.mForwNodeId;
    mReveNodeId = other.mReveNodeId;
    mForwWeight = other.mForwWeight;
    mReveWeight = other.mReveWeight;
    mBeforePNode = other.mBeforePNode;
    mAfterPNode = other.mAfterPNode;
    mNetworkNNode = other.mNetworkNNode;
    mBearing = other.mBearing;
    // mStreetName = other.mStreetName;
    // this->setStreetName( other.streetName() );
    // mStreetName.assign(other.mStreetName);
    mNameId = other.mNameId;
    return *this;
}
