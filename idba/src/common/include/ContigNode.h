#ifndef __CONTIG_NODE_H_

#define __CONTIG_NODE_H_

#include "Kmer.h"
#include "Contig.h"
#include "AbstractNode.h"


class ContigNode: public AbstractNode
{
public:
    ContigNode() { }
    ContigNode(const Contig &contig) 
    { SetContent(contig); }

    void SetContent(const Contig &contig)
    { this->contig = contig; Clear(); }

    void Swap(ContigNode &node)
    {
        if (this != &node)
        {
            AbstractNode::Swap(node);
            contig.Swap(node.contig);
        }
    }

//    void ReverseComplement()
//    {
//        AbstractNode::ReverseComplement();
//        contig.ReverseComplement();
//    }

    void Merge(ContigNode *node)
    {
        contig.Merge(node->contig);
        SetOutEdges(node->OutEdges());
    }

    Kmer GetBeginKmer() const
    { return contig.GetBeginKmer(); }
    Kmer GetEndKmer() const 
    { return contig.GetEndKmer(); }

    const Contig &GetContig() const { return contig; }
    int GetSize() const { return contig.Size(); }

private:
    Contig contig;
};

class ContigNodeAdapter: public AbstractNodeAdapter
{
public:
    ContigNodeAdapter(ContigNode *node = NULL, bool is_reverse = false) 
    { SetNode(node, is_reverse); }

    void SetNode(ContigNode *node, bool is_reverse = false)
    { AbstractNodeAdapter::SetNode(node, is_reverse); }

    const ContigNode *GetNode() const { return (const ContigNode*)node; }
    ContigNode *GetNode() { return (ContigNode*)node; }

    Kmer GetBeginKmer() const
    {
        if (!is_reverse)
        {
            return GetNode()->GetContig().GetBeginKmer();
        }
        else
        {
            Kmer kmer = GetNode()->GetContig().GetEndKmer();
            kmer.ReverseComplement();
            return kmer;
        }
    }

    Kmer GetEndKmer() const
    {
        if (!is_reverse)
        {
            return GetNode()->GetContig().GetEndKmer();
        }
        else
        {
            Kmer kmer = GetNode()->GetContig().GetBeginKmer();
            kmer.ReverseComplement();
            return kmer;
        }
    }

    unsigned GetNucleotide(int index) const
    {
        if (!is_reverse)
            return GetNode()->GetContig()[index];
        else
            return 3 - GetNode()->GetContig()[GetNode()->GetContig().Size()-1 - index];

    }

    void GetContig(Contig &contig) const
    {
        contig = GetNode()->GetContig();
        if (is_reverse)
            contig.ReverseComplement();
    }

    int GetSize() const
    { return GetNode()->GetSize(); }


//    Contig GetContig()
//    {
//        if (!is_reverse)
//        {
//            return GetNode()->GetContig();
//        }
//        else
//        {
//            Contig contig = GetNode()->GetContig();
//            contig.ReverseComplement();
//            return contig;
//        }
//    }
};

#endif
