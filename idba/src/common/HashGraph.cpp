#include "globals.h"
#include "Kmer.h"
#include "Sequence.h"
#include "Contig.h"
#include "HashNode.h"
#include "HashGraph.h"
#include "Log.h"
#include "BranchGroup.h"

#include <omp.h>

#include <cstdio>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <vector>
#include <queue>

using namespace std;

static const uint64 UnitOne = 100000000ULL;

void HashGraph::ClearGraph()
{
    for (unsigned i = 0; i < table_size; ++i)
    {
        for (HashNode *node = table[i]; node != NULL; node = node->next)
        {
            node->Clear();
            node->SetCount(1);
        }
    }
    num_edges = 0;
}

void HashGraph::InsertSequence(const Sequence &seq, uint64 prefix, uint64 mask)
{
    if (seq.Size() < kmerLength)
        return;

    Kmer kmer;
    for (int i = 0; i < kmerLength-1; ++i)
        kmer.AddRight(seq[i]);
    for (int i = kmerLength-1; i < seq.Size(); ++i)
    {
        kmer.AddRight(seq[i]);
        Kmer key = kmer;
        Kmer rev_comp = kmer;
        rev_comp.ReverseComplement();
        if (rev_comp < kmer)
            key = rev_comp;

        if ((key.Hash() & mask) == prefix)
        {
            KmerNodeAdapter adp(InsertKmer(kmer), kmer);
            if (i >= (int)kmerLength)
            {
                adp.AddInEdge(3 - seq[i-kmerLength]);
            }

            if (i+1 < seq.Size())
            {
                adp.AddOutEdge(seq[i+1]);
            }
        }
    }
}

void HashGraph::AddInternalKmers(const Sequence &seq, int minCount)
{
    if (seq.Size() <= kmerLength)
        return;

    vector<int> v;
    int count = 0;
    int sum = 0;
    Kmer kmer;
    for (int i = 0; i < kmerLength-1; ++i)
        kmer.AddRight(seq[i]);
    for (int i = kmerLength-1; i < seq.Size(); ++i)
    {
        kmer.AddRight(seq[i]);

        KmerNode *node = GetNode(kmer);
        if (node != NULL && node->Count() >= (unsigned)minCount)
        {
            sum += node->Count();
            ++count;
            v.push_back(i);
        }
    }

    if (count > max(seq.Size() - kmerLength*2 + 1, (seq.Size() - kmerLength + 1)/2))
    {
        Kmer kmer;
        for (int i = 0; i < kmerLength-1; ++i)
            kmer.AddRight(seq[i]);
        for (int i = kmerLength-1; i < seq.Size(); ++i)
        {
            kmer.AddRight(seq[i]);

            if (v.front() <= i && i <= v.back() && GetNode(kmer) == NULL)
            {
                KmerNodeAdapter adp(InsertKmer(kmer), kmer);
                if (i >= (int)kmerLength)
                {
                    adp.AddInEdge(3 - seq[i-kmerLength]);
                }

                if (i+1 < seq.Size())
                {
                    adp.AddOutEdge(seq[i+1]);
                }
            }
        }
    }
}

void HashGraph::AddEdgesAndInternalKmers(const Sequence &seq, int minCount)
{
    if (seq.Size() <= kmerLength)
        return;

    vector<int> v;
    int count = 0;
    int sum = 0;
    Kmer kmer;
    for (int i = 0; i < kmerLength-1; ++i)
        kmer.AddRight(seq[i]);
    for (int i = kmerLength-1; i < seq.Size(); ++i)
    {
        kmer.AddRight(seq[i]);

        KmerNode *node = GetNode(kmer);
        if (node != NULL && node->Count() >= (unsigned)minCount)
        {
            sum += node->Count();
            ++count;
            v.push_back(i);
            node->Increase();

            KmerNodeAdapter adp(node, kmer);
            if (i >= (int)kmerLength)
            {
                adp.AddInEdge(3 - seq[i-kmerLength]);
            }

            if (i+1 < seq.Size())
            {
                adp.AddOutEdge(seq[i+1]);
            }
        }
    }

    if (count > max(seq.Size() - kmerLength*2 + 1, (seq.Size() - kmerLength + 1)/2))
    {
        Kmer kmer;
        for (int i = 0; i < kmerLength-1; ++i)
            kmer.AddRight(seq[i]);
        for (int i = kmerLength-1; i < seq.Size(); ++i)
        {
            kmer.AddRight(seq[i]);

            if (v.front() <= i && i <= v.back() && GetNode(kmer) == NULL)
            {
                KmerNodeAdapter adp(InsertKmer(kmer), kmer);

                if (i >= (int)kmerLength)
                {
                    adp.AddInEdge(3 - seq[i-kmerLength]);
                }

                if (i+1 < seq.Size())
                {
                    adp.AddOutEdge(seq[i+1]);
                }
            }
        }
    }
}

bool HashGraph::IsValid(const Sequence &seq)
{
    Kmer kmer;
    for (int i = 0; i < kmerLength-1; ++i)
        kmer.AddRight(seq[i]);
    for (int i = kmerLength-1; i < seq.Size(); ++i)
    {
        kmer.AddRight(seq[i]);
        if (GetNode(kmer) == NULL)
            return false;
    }

    return true;
}

void HashGraph::AddAllEdges()
{
    num_edges = 0;
#pragma omp parallel for
    for (int64 i = 0; i < (int64)table_size; ++i)
    {
        for (HashNode *node = table[i]; node; node = node->next)
        {
            node->SetInEdges(15);
            node->SetOutEdges(15);
        }
    }
}

bool HashGraph::AddEdgesFromSequence(const Sequence &seq)
{
    if (seq.Size() < kmerLength)
        return false;

    bool flag = false;
    Kmer kmer;
    for (int i = 0; i < kmerLength-1; ++i)
        kmer.AddRight(seq[i]);
    for (int i = kmerLength-1; i < seq.Size(); ++i)
    {
        kmer.AddRight(seq[i]);

        KmerNodeAdapter adp = GetNodeAdapter(kmer);
        if (!adp.IsNull())
        {
            flag = true;
            adp.Increase();
            if (i >= (int)kmerLength)
            {
                adp.AddInEdge(3 - seq[i-kmerLength]);
            }

            if (i+1 < seq.Size())
            {
                adp.AddOutEdge(seq[i+1]);
            }
        }
    }

    return flag;
}

int64 HashGraph::RemoveDeadEnd(unsigned minLength)
{
    int64 total = 0;
    for (int i = 3; i >= 0; --i)
    {
        int64 trimmed = Trim(minLength >> i);
        total += trimmed;
    }

    return total;
}

void HashGraph::Refresh(unsigned minCount)
{
    RefreshVertices(minCount);
    RefreshEdges();
}

void HashGraph::RefreshVertices(unsigned minCount)
{
#pragma omp parallel for
    for (int64 i = 0; i < (int64)table_size; ++i)
    {
        HashNode *node = table[i];
        HashNode *prev = NULL;
        while (node != NULL)
        {
            if (node->IsDead() || node->Count() < minCount)
            {
#pragma omp atomic
                --num_nodes;
                if (prev == NULL)
                {
                    table[i] = node->next;
                    FreeNode(node, omp_get_thread_num());
                    node = table[i];
                }
                else
                {
                    prev->next = node->next;
                    FreeNode(node, omp_get_thread_num());
                    node = prev->next;
                }
            }
            else
            {
                node->ClearStatus();
                prev = node;
                node = prev->next;
            }
        }
    }
}

void HashGraph::RefreshEdges()
{
    num_edges = 0;
#pragma omp parallel for
    for (int64 i = 0; i < (int64)table_size; ++i)
    {
        for (HashNode *node = table[i]; node; node = node->next)
        {
            KmerNodeAdapter curr(node);
            for (int strand = 0; strand < 2; ++strand)
            {
                Kmer kmer;
                curr.GetKmer(kmer);
                unsigned edges = curr.OutEdges();
                for (int x = 0; x < 4; ++x)
                {
                    if (edges & (1 << x))
                    {
                        Kmer next = kmer;
                        next.AddRight(x);
                        if (GetNode(next) == NULL)
                            curr.RemoveOutEdge(x);
                        else
                        {
#pragma omp atomic
                            ++num_edges;
                        }
                    }
                }

                curr.ReverseComplement();
            }

            if (node->kmer.IsPalindrome())
            {
                unsigned edges = node->InEdges() | node->OutEdges();
                node->SetInEdges(edges);
                node->SetOutEdges(edges);
            }
        }
    }

    num_edges >>= 1;
}

bool HashGraph::Check()
{
    for (int64 i = 0; i < (int64)table_size; ++i)
    {
        HashNode *node = table[i];
        while (node != NULL)
        {
            KmerNodeAdapter adapter(node);
            Kmer kmer = adapter.GetNode()->GetKmer();
            for (int strand = 0; strand < 2; ++strand)
            {
                unsigned edges = adapter.OutEdges();

                for (int x = 0; x < 4; ++x)
                {
                    if (edges & (1 << x))
                    {
                        Kmer next = kmer;
                        next.AddRight(x);
                        KmerNode *q = GetNode(next);
                        if (q == NULL)
                        {
                            cout << "null fail" << endl;
                            return false;
                        }

                        if (q->IsDead())
                        {
                            cout << "deadend fail" << endl;
                            return false;
                        }

                        KmerNodeAdapter adp(q, next);

                        if (((1 << (3 - kmer.GetBase(0))) & adp.InEdges()) == 0)
                        {
                            cout << (int)kmer.GetBase(0) << " " << (int)adp.InEdges() << endl;
                            cout << "no in edge fail" << endl;
                            return false;
                        }
                    }
                }

                kmer.ReverseComplement();
                adapter.ReverseComplement();
            }

            node = node->next;
        }
    }

    return true;
}

int64 HashGraph::Trim(int minLength)
{
    vector<Contig> contigs;
    Assemble(contigs);

    int total = 0;
#pragma omp parallel for
    for (int i = 0; i < (int)contigs.size(); ++i)
    {
        if (contigs[i].IsTangle() && contigs[i].Size() < kmerLength + minLength - 1)
        {
            Kmer kmer;
            for (int j = 0; j+1 < kmerLength; ++j)
                kmer.AddRight(contigs[i][j]);
            for (int j = kmerLength-1; j < contigs[i].Size(); ++j)
            {
                kmer.AddRight(contigs[i][j]);
                KmerNode *node = GetNode(kmer);
                if (node != NULL)
                    node->SetDeadFlag();
            }

#pragma omp atomic
            ++total;
        }
    }

    Refresh();

    LogMessage("trim %lld dead ends\n", total);

    return total;
}

int64 HashGraph::RemoveBubble()
{
    unsigned bubble = 0;
//#pragma omp parallel for
    for (unsigned i = 0; i < table_size; ++i)
    {
        for (HashNode *node = table[i]; node != NULL; node = node->next)
        {
            KmerNodeAdapter adp(node);
            for (int strand = 0; strand < 2; ++strand)
            {
                if (adp.InDegree() == 1 && adp.OutDegree() == 2)
                {
                    BranchGroup branch_group(this, adp, 2, kmerLength + 2);
                    if (branch_group.Search())
                    {
                        branch_group.Merge();
                        ++bubble;
                    }
                }
                adp.ReverseComplement();
            }
        }
    }

    Refresh();

    return bubble;
}

double HashGraph::AverageCoverage()
{
    long long sum = 0;
    uint64 valid = 0;
    for (int64 i = 0; i < table_size; ++i)
    {
        for (HashNode *node = table[i]; node; node = node->next)
        {
            sum += node->Count();
            ++valid;
        }
    }

    return 1.0 * sum / valid;
}

double HashGraph::MedianCoverage()
{
    vector<int> v;
    v.reserve(num_nodes);
    for (int64 i = 0; i < table_size; ++i)
    {
        for (HashNode *node = table[i]; node; node = node->next)
        {
            v.push_back(node->Count());
        }
    }

    nth_element(v.begin(), v.begin() + v.size()/2, v.end());

    return *(v.begin() + v.size()/2);
}

int64 HashGraph::RemoveLowCoverageContigs(double c)
{
    vector<Contig> contigs;
    Assemble(contigs);

    int total = 0;
#pragma omp parallel for
    for (int i = 0; i < (int)contigs.size(); ++i)
    {
        if (contigs[i].Coverage() < c)
        {
            Kmer kmer;
            for (int j = 0; j+1 < kmerLength; ++j)
                kmer.AddRight(contigs[i][j]);
            for (int j = kmerLength-1; j < contigs[i].Size(); ++j)
            {
                kmer.AddRight(contigs[i][j]);
                KmerNode *node = GetNode(kmer);
                if (node != NULL)
                    node->SetDeadFlag();
            }

#pragma omp atomic
            ++total;
        }
    }

    Refresh();

    return total;
}

int64 HashGraph::Assemble(vector<Contig> &contigs)
{
    contigs.resize(0);
    int tangle = 0;

    omp_lock_t lockContigs;
    omp_init_lock(&lockContigs);

#pragma omp parallel for
    for (int64 i = 0; i < (int64)table_size; ++i)
    {
        for (HashNode *node = table[i]; node != NULL; node = node->next)
        {
            if (!node->Lock(omp_get_thread_num()))
                continue;

            Contig contig;
            contig.SetContent(node->kmer, kmerLength);
            contig.sum_coverage = node->Count();
            contig.is_tangle = false;

            if (!node->kmer.IsPalindrome())
            {
                for (int strand = 0; strand < 2; ++strand)
                {
                    KmerNodeAdapter adp = GetNodeAdapter(contig.GetEndKmer());
                    KmerNodeAdapter next(NULL);

                    while (true)
                    {
                        if (!GetNextNodeAdapter(adp, next))
                            break;

                        if (next.GetLockID() == omp_get_thread_num() && IsLoop(contig, next))
                            break;

                        if (!next.LockPreempt(omp_get_thread_num()))
                            goto FAIL;

                        contig.AddNucleotide(BitOperation::bitToIndex[adp.OutEdges()]);
                        adp = next;
                        contig.sum_coverage += next.Count();
                    }

                    if (adp.OutDegree() == 0)
                        contig.is_tangle = true;
                    contig.ReverseComplement();
                }
            }

            if (contig.is_tangle)
            {
#pragma omp atomic
                ++tangle;
            }

            omp_set_lock(&lockContigs);
            contigs.push_back(contig);
            omp_unset_lock(&lockContigs);
FAIL:
            ;
        }
    }

    omp_destroy_lock(&lockContigs);

    ClearStatus();
    LogMessage("tangle %d total %d\n", tangle, contigs.size());

    return contigs.size();

}

