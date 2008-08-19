
#include <ql/models/marketmodels/pathwisegreeks/vegabumpcluster.hpp>
#include <ql/errors.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>

namespace
QuantLib
{



    VegaBumpCluster::VegaBumpCluster(Size factorBegin,
        Size factorEnd,
        Size rateBegin,
        Size rateEnd,
        Size stepBegin,
        Size stepEnd)
        :
    factorBegin_(factorBegin),
        factorEnd_(factorEnd),
        rateBegin_(rateBegin),
        rateEnd_(rateEnd),
        stepBegin_(stepBegin),
        stepEnd_(stepEnd)
    {
        QL_REQUIRE(factorBegin_<factorEnd_, "must have factorBegin_ < factorEnd_ in VegaBumpCluster ");
        QL_REQUIRE(rateBegin_<rateEnd_, "must have rateBegin_ < rateEnd_ in VegaBumpCluster ");
        QL_REQUIRE(stepBegin_<stepEnd_, "must have stepBegin_ < stepEnd_ in VegaBumpCluster ");
    }

    bool VegaBumpCluster::doesIntersect(const VegaBumpCluster& comparee) const
    {
        if (factorEnd_ <= comparee.factorBegin_)
            return false;

        if (rateEnd_ <= comparee.rateBegin_)
            return false;

        if (stepEnd_ <= comparee.stepBegin_)
            return false;


        if (comparee.factorEnd_ <= factorBegin_)
            return false;

        if (comparee.rateEnd_ <= rateBegin_)
            return false;

        if (comparee.stepEnd_ <= stepBegin_)
            return false;

        return true;


    }

        
    bool VegaBumpCluster::isCompatible(const boost::shared_ptr<MarketModel>& volStructure) const
    {


        if (rateEnd_ > volStructure->numberOfRates())
            return false;

        if (stepEnd_ > volStructure->numberOfSteps())
            return false;

        if (factorEnd_ > volStructure->numberOfFactors())
            return false;

        Size firstAliveRate = volStructure->evolution().firstAliveRate()[stepEnd_-1];

        if (rateBegin_ < firstAliveRate) // if the rate has reset before the beginning of the last step of the bump
            return false; 

        return true;

    }



    VegaBumpCollection::VegaBumpCollection(const boost::shared_ptr<MarketModel>& volStructure, 
                           bool factorwiseBumping) 
                            : associatedVolStructure_(volStructure)
    {
        Size steps = volStructure->numberOfSteps();
        Size rates = volStructure->numberOfRates();
        Size factors = volStructure->numberOfFactors();

        for (Size s=0; s < steps; ++s)
            for (Size r=volStructure->evolution().firstAliveRate()[s]; r < rates; ++r)
            {
                if (factorwiseBumping)
                {
                    for (Size f=0; f < factors; ++f)
                    {
                        VegaBumpCluster thisCluster(f,f+1,r,r+1,s,s+1);
                        allBumps_.push_back(thisCluster);

                    }
                }
                else
                {
                     VegaBumpCluster thisCluster(0,factors,r,r+1,s,s+1);
                     allBumps_.push_back(thisCluster);

                }
            }

        checked_=true;
        full_=true;
        nonOverlapped_=true;


    }
         

    VegaBumpCollection::VegaBumpCollection(const std::vector<VegaBumpCluster>& allBumps,  const boost::shared_ptr<MarketModel>& volStructure)
        : allBumps_(allBumps), associatedVolStructure_(volStructure), checked_(false)
    {
        for (Size j=0; j < allBumps_.size(); ++j)
            QL_REQUIRE(allBumps_[j].isCompatible(associatedVolStructure_),"incompatible bumps passed to VegaBumpCollection");
        
    }
    

    const std::vector<VegaBumpCluster>& VegaBumpCollection::allBumps() const
    {
        return allBumps_;
    }

    bool VegaBumpCollection::isFull() const // i.e. is every alive pseudo-root element bumped at least once
    {
        if (checked_)
            return full_;
        std::vector<std::vector<std::vector<bool> > > v;

        Size factors = associatedVolStructure_->numberOfFactors();

        std::vector<bool> model(factors);
        std::fill(model.begin(), model.end(), false);

        std::vector<std::vector<bool> > modelTwo;
        for (Size i=0; i < associatedVolStructure_->numberOfRates(); ++i)
            modelTwo.push_back(model);

        for (Size j=0; j < associatedVolStructure_->numberOfSteps(); ++j)
            v.push_back(modelTwo);

        for (Size k=0; k < allBumps_.size(); ++k)
        {            
            for (Size f=allBumps_[k].factorBegin(); f <  allBumps_[k].factorEnd(); ++f)
                for (Size r=allBumps_[k].rateBegin(); r <  allBumps_[k].rateEnd(); ++r)
                    for (Size s= allBumps_[k].stepBegin(); s <  allBumps_[k].stepEnd(); ++s)
                        v[s][r][f] = true;

        }

        Size numberFailures =0;
        for (Size s =0; s < associatedVolStructure_->numberOfSteps(); ++s)
            for (Size f=0; f < associatedVolStructure_->numberOfFactors(); ++f)
                for (Size r=associatedVolStructure_->evolution().firstAliveRate()[s]; r <  associatedVolStructure_->numberOfRates(); ++r)
                    if (!v[s][r][f])
                        ++numberFailures;

        return numberFailures>0;

    }
    
    bool VegaBumpCollection::isNonOverlapping() const // i.e. is every alive pseudo-root element bumped at most once
    {
        
        if (checked_)
            return nonOverlapped_;

        std::vector<std::vector<std::vector<bool> > > v;
   
        Size factors = associatedVolStructure_->numberOfFactors();

      
        std::vector<bool> model(factors);
        std::fill(model.begin(), model.end(), false);

        std::vector<std::vector<bool> > modelTwo;
        for (Size i=0; i < associatedVolStructure_->numberOfRates(); ++i)
            modelTwo.push_back(model);

        for (Size j=0; j < associatedVolStructure_->numberOfSteps(); ++j)
            v.push_back(modelTwo);
        
        Size numberFailures=0;

        for (Size k=0; k < allBumps_.size(); ++k)
        {            
            for (Size f=allBumps_[k].factorBegin(); f <  allBumps_[k].factorEnd(); ++f)
                for (Size r=allBumps_[k].rateBegin(); r <  allBumps_[k].rateEnd(); ++r)
                    for (Size s= allBumps_[k].stepBegin(); s <  allBumps_[k].stepEnd(); ++s)
                    {
                        if (v[s][r][f])
                            ++numberFailures;
                        v[s][r][f] = true;
                    }

        }

        return numberFailures>0;

    }

    bool VegaBumpCollection::isSensible() const // i.e. is every alive pseudo-root element bumped precisely once
    {
        if (checked_)
            return true;

        return isNonOverlapping() && isFull();
    }

    
    Size VegaBumpCollection::numberBumps() const
    {
        return allBumps_.size();
    }

}