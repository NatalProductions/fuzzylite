/*
 * Sigmoid.h
 *
 *  Created on: 30/11/2012
 *      Author: jcrada
 */

#ifndef FL_SIGMOID_H_
#define FL_SIGMOID_H_

#include "fl/term/Term.h"

namespace fl {

    class Sigmoid : public Term {
    protected:
        scalar _inflection;
        scalar _slope;
        scalar _minimum, _maximum;
    public:
        Sigmoid(const std::string& name = "",
                scalar inflection = std::numeric_limits<scalar>::quiet_NaN(),
                scalar slope = std::numeric_limits<scalar>::quiet_NaN(),
                scalar minimum = -std::numeric_limits<scalar>::infinity(),
                scalar maximum = std::numeric_limits<scalar>::infinity());
        ~Sigmoid();

        std::string className() const;
        Sigmoid* copy() const;

        scalar membership(scalar x) const;

        std::string toString() const;

        void setInflection(scalar inflection);
        scalar getInflection() const;

        void setSlope(scalar slope);
        scalar getSlope() const;



        void setMinimum(scalar minimum);
        scalar minimum() const;

        void setMaximum(scalar maximum);
        scalar maximum() const;

    };

} /* namespace fl */
#endif /* FL_SIGMOID_H_ */
