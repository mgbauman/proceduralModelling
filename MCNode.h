#pragma once
class MCNode
{
public:

    float in, out, level;
    MCNode* innode;
    MCNode* outnode;
    MCNode* levelnode;
    MCNode::MCNode(float inprob, float outprob, float levelprob, MCNode* innode, MCNode* outnode, MCNode* levelnode);
    MCNode::MCNode();
    MCNode::MCNode(float inprob, float outprob, float levelprob);
    MCNode::~MCNode();
};

