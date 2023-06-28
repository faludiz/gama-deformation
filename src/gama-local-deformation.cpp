#include <iostream>
#include <iomanip>
#include <fstream>
#include <memory>
#include <map>

#include <gnu_gama/xml/localnetwork_adjustment_results.h>
using Results = GNU_gama::LocalNetworkAdjustmentResults;

#define DEBUG

struct Rec {
    std::string id;
    int indx {0}; double dx {0};
    int indy {0}; double dy {0};
    int indz {0}; double dz {0};

    //int dim {0};
};

struct Rec2 {
    std::string id;
    int indx1 {0}; double x1 {0};
    int indy1 {0}; double y1 {0};
    int indz1 {0}; double z1 {0};
    int indx2 {0}; double x2 {0};
    int indy2 {0}; double y2 {0};
    int indz2 {0}; double z2 {0};

    bool empty() const { return dim() == 0; }

    int dim() const {
        int d = 0;
        if (indz1*indz2) d += 1;
        if (indx1*indx2) d += 2;
        return d;
    }

#ifdef DEBUG
    friend std::ostream& operator<<(std::ostream& os, const Rec2& rec);
#endif
};

#ifdef DEBUG
std::ostream& operator<<(std::ostream& os, const Rec2& rec)
{
    os << "  " << rec.id << "  "
       << rec.indx1 << " " << rec.x1 << " "
       << rec.indy1 << " " << rec.y1 << " "
       << rec.indz1 << " " << rec.z1 << "\t "
       << rec.indx2 << " " << rec.x2 << " "
       << rec.indy2 << " " << rec.y2 << " "
       << rec.indz2 << " " << rec.z2 << "\n"
        ;
    return os;
}
#endif


int main(int argc, char *argv[])
{
    std::map<std::string, Rec2> adjrec;

    auto epoch1 = std::make_unique<Results>();
    {
        std::ifstream inp_epoch1(argv[1]);
        if (!inp_epoch1) {
            std::cout << "   ####  ERROR ON OPENING 1st EPOCH ADJUSTMENT FILE "
                      << argv[1] << "\n";
            return 1;
        }
        epoch1->read_xml(inp_epoch1);
    }

    for (const auto& p1 : epoch1->adjusted_points)
    {
        auto& rec = adjrec[p1.id];
        rec.id = p1.id; // p1.cxy p1.cz constrained
        rec.indx1 = p1.indx; rec.x1 = p1.x;
        rec.indy1 = p1.indy; rec.y1 = p1.y;
        rec.indz1 = p1.indz; rec.z1 = p1.z;
    }

#ifdef DEBUG
    for (const auto& r : adjrec) std::cout << r.second;
    std::cout << epoch1->cov;
#endif


    auto epoch2 = std::make_unique<Results>();
    {
        std::ifstream inp_epoch2(argv[2]);
        if (!inp_epoch2) {
            std::cout << "   ####  ERROR ON OPENING 2nd EPOCH ADJUSTMENT FILE "
                      << argv[2] << "\n";
            return 1;
        }
        epoch2->read_xml(inp_epoch2);
    }

    for (const auto& p2 : epoch2->adjusted_points)
    {
        auto& rec = adjrec[p2.id];
        rec.id = p2.id; // p1.cxy p1.cz constrained
        rec.indx2 = p2.indx; rec.x2 = p2.x;
        rec.indy2 = p2.indy; rec.y2 = p2.y;
        rec.indz2 = p2.indz; rec.z2 = p2.z;
    }

#ifdef DEBUG
    for (const auto& r : adjrec) std::cout << r.second;
    std::cout << epoch2->cov;
#endif


    int adjcov_dim = 0;
    for (const auto& r : adjrec) adjcov_dim += r.second.dim();


#ifdef DEBUG

    // std::cout << adjcov << "\n";
#endif

    std::vector<int> t1 {0}, t2 {0}; // 1 based index transformation
    for (const auto& r : adjrec)
    {
        if (r.second.indx1 && r.second.indx2) {
            t1.push_back( r.second.indx1 );
            t1.push_back( r.second.indy1 );

            t2.push_back( r.second.indx2 );
            t2.push_back( r.second.indy2 );
        }
        if (r.second.indz1 && r.second.indz2) {
            t1.push_back( r.second.indz1 );
            t2.push_back( r.second.indz2 );
        }
    }

#ifdef DEBUG
    std::cout << "\nadjcov_dim = " << adjcov_dim << "\n";

    std::cout << "t1 = ";
    for (int i=1; i<=adjcov_dim; i++) {
        std::cout << t1[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "t2 = ";
    for (int i=1; i<=adjcov_dim; i++) {
        std::cout << t2[i] << " ";
    }
    std::cout << "\n\n";
#endif

    int cov_index {0};
    std::map<std::string, Rec> adjdiff;
    for (auto& adjr : adjrec)
        if (!adjr.second.empty())
        {
            std::string id = adjr.second.id;
            Rec rec;
            rec.id  = id;
            rec.dx  = adjr.second.x2 - adjr.second.x1;
            rec.dy  = adjr.second.y2 - adjr.second.y1;
            rec.dz  = adjr.second.z2 - adjr.second.z1;
            // rec.dim = adjr.second.dim();

            if (adjr.second.dim() == 3) {
                rec.indx = ++cov_index;
                rec.indy = ++cov_index;
                rec.indz = ++cov_index;
            }
            else if (adjr.second.dim() == 2) {
                rec.indx = ++cov_index;
                rec.indy = ++cov_index;
            }
            else if (adjr.second.dim() == 1) {
                rec.indz = ++cov_index;
            }

            adjdiff[id] = rec;
        }

    for (auto& rec : adjdiff)
    {
        auto& r = rec.second;
        std::cout << r.id << "  "
                  << r.indx << " " << r.indy << " " << r.indz << " "
                  << r.dx << " " << r.dy << " " << r.dz << "\n";
    }
}


