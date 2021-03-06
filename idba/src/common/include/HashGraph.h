#ifndef __HASH_GRAPH_H_

#define __HASH_GRAPH_H_

#include "globals.h"
#include "Kmer.h"
#include "HashNode.h"
#include "Sequence.h"
#include "Contig.h"
#include "HashTable.h"

#include <omp.h>

#include <algorithm>
#include <iostream>
#include <vector>


class Kmer;

class HashGraph: public HashTable
{
public:
    friend class BranchGroup;
    HashGraph(int64 table_size = MaxHashTable) : HashTable(table_size, true) {}
    ~HashGraph() {}

    void Clear() { HashTable::Clear(); num_edges = 0; }
    void ClearGraph();

    int64 NumEdges() const { return num_edges; }
    KmerNodeAdapter GetNodeAdapter(const Kmer &kmer) { return KmerNodeAdapter(GetNode(kmer), kmer); }

    void InsertSequence(const Sequence &seq, uint64 prefix = 0, uint64 mask = 0);
    void AddInternalKmers(const Sequence &seq, int minCount = 0);
    void AddEdgesAndInternalKmers(const Sequence &seq, int minCount = 0);
    bool IsValid(const Sequence &seq);

    void AddAllEdges();
    bool AddEdgesFromSequence(const Sequence &seq);

    int64 RemoveDeadEnd(unsigned minCount);

    void Refresh(unsigned minCount = 0);
    void RefreshVertices(unsigned minCount = 0);
    void RefreshEdges();

    bool Check();

    int64 Trim(int minLength);
    int64 RemoveBubble();

    double AverageCoverage();
    double MedianCoverage();

    int64 RemoveLowCoverageContigs(double c);
    int64 Assemble(std::vector<Contig> &contigs);

private:
    HashGraph(const HashGraph &);
    const HashGraph &operator =(const HashGraph);

    bool GetNextNodeAdapter(KmerNodeAdapter &current, KmerNodeAdapter &next)
    {
        if (current.OutDegree() != 1)
            return false;

        Kmer kmer;
        current.GetKmer(kmer);
        kmer.AddRight(BitOperation::bitToIndex[current.OutEdges()]);
        next = GetNodeAdapter(kmer);

        return !kmer.IsPalindrome() && next.InDegree() == 1;
    }
    
    bool IsLoop(Contig &contig, KmerNodeAdapter &next)
    {
        Kmer kmer;
        next.GetKmer(kmer);
        Kmer rev_comp = kmer;
        rev_comp.ReverseComplement();
        return contig.GetBeginKmer() == kmer || contig.GetEndKmer() == rev_comp;
    }

    int64 num_edges;
};

#endif
