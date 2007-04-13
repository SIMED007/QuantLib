/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Mark Joshi
 Copyright (C) 2007 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#ifndef quantlib_forward_to_coterminal_adapter_hpp
#define quantlib_forward_to_coterminal_adapter_hpp

#include <ql/marketmodels/marketmodel.hpp>

namespace QuantLib {

    class EvolutionDescription;

    class ForwardToCoterminalAdapter : public MarketModel {
      public:
        ForwardToCoterminalAdapter(
                          const boost::shared_ptr<MarketModel>& forwardModel);
        //! \name MarketModel interface
        //@{
        const std::vector<Rate>& initialRates() const;
        const std::vector<Spread>& displacements() const;
        const EvolutionDescription& evolution() const;
        Size numberOfRates() const;
        Size numberOfFactors() const;
        Size numberOfSteps() const;
        const Matrix& pseudoRoot(Size i) const;
        //@}
      private:
        boost::shared_ptr<MarketModel> fwdModel_;
        Size numberOfFactors_, numberOfRates_, numberOfSteps_;
        std::vector<Rate> initialRates_;
        std::vector<Matrix> pseudoRoots_;
    };


    class ForwardToCoterminalAdapterFactory : public MarketModelFactory,
                                              public Observer {
      public:
        ForwardToCoterminalAdapterFactory(
              const boost::shared_ptr<MarketModelFactory>& forwardFactory);
        boost::shared_ptr<MarketModel> create(const EvolutionDescription&,
                                              Size numberOfFactors) const;
        void update();
      private:
        boost::shared_ptr<MarketModelFactory> forwardFactory_;
    };


    // inline definitions

    inline const std::vector<Rate>&
    ForwardToCoterminalAdapter::initialRates() const {
        return initialRates_;
    }

    inline const std::vector<Spread>&
    ForwardToCoterminalAdapter::displacements() const {
        return fwdModel_->displacements();
    }

    inline const EvolutionDescription&
    ForwardToCoterminalAdapter::evolution() const {
        return fwdModel_->evolution();
    }

    inline Size ForwardToCoterminalAdapter::numberOfRates() const {
        return fwdModel_->numberOfRates();
    }

    inline Size ForwardToCoterminalAdapter::numberOfFactors() const {
        return fwdModel_->numberOfFactors();
    }

    inline Size ForwardToCoterminalAdapter::numberOfSteps() const {
        return fwdModel_->numberOfSteps();
    }

    inline const Matrix& ForwardToCoterminalAdapter::pseudoRoot(Size i) const {
        return pseudoRoots_[i];
    }

}

#endif