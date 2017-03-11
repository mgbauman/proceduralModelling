#include "MCNode.h"



MCNode::MCNode(float inprob, float outprob, float levelprob, MCNode* in, MCNode* out, MCNode* level)
{
    this->in = inprob;
    this->out = outprob;
    this->level = levelprob;
    this->innode = in;
    this->outnode = out;
    this->levelnode = level;
}

MCNode::MCNode(float inprob, float outprob, float levelprob)
{
    this->in = inprob;
    this->out = outprob;
    this->level = levelprob;
}

MCNode::MCNode()
{

}


MCNode::~MCNode()
{
}
