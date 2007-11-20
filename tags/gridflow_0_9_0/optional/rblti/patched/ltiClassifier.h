/*
 * Copyright (C) 1998, 1999, 2000, 2001
 * Lehrstuhl fuer Technische Informatik, RWTH-Aachen, Germany
 *
 * This file is part of the LTI-Computer Vision Library (LTI-Lib)
 *
 * The LTI-Lib is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License (LGPL)
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * The LTI-Lib is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the LTI-Lib; see the file LICENSE.  If
 * not, write to the Free Software Foundation, Inc., 59 Temple Place -
 * Suite 330, Boston, MA 02111-1307, USA.
 */


/*----------------------------------------------------------------
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltiClassifier.h
 * authors ....: Pablo Alvarado, Peter Doerfler
 * organization: LTI, RWTH Aachen
 * creation ...: 10.8.2000
 * revisions ..: $Id$
 */

#ifndef _LTI_CLASSIFIER_H_
#define _LTI_CLASSIFIER_H_

#ifndef _INCLUDE_DEPRECATED
#define _INCLUDE_DEPRECATED 1
#endif

#include "ltiObject.h"
#include <list>
#include <map>
#include <vector>

#include "ltiIoObject.h"
#include "ltiException.h"
#include "ltiVector.h"
#include "ltiMatrix.h"
#include "ltiIoHandler.h"
#include "ltiProgressInfo.h"
#include "ltiStatus.h"

# ifdef min
#   undef min
# endif

# ifdef max
#   undef max
# endif

namespace lti {


  /**
   * Abstract parent class for all classifiers.
   *
   * The classifier class encloses a number of other important classes:
   * <dl>
   * <dt>lti::classifier::parameters</dt>
   * <dd>similar to the functor::parameters, this is used to specify the
   *     characteristics of a classifier like learning rate, etc.</dd>
   * <dt>lti::classifier::outputVector</dt>
   * <dd>This type is used to return the classification results.</dd>
   * <dt>lti::classifier::outputTemplate</dt>
   * <dd>Contains information on how the outputVector is to be constructed
   *     from a resulting vector of the classification.</dd>
   * </dl>
   *
   * Classifiers can be diveded into two large groups: supervised and
   * unsupervised. These two groups have the base classes
   * supervisedClassifier and unsupervisedClassifier which are
   * subclasses of this class. Examples for the first type are RBF
   * networks, multi layer perceptron, maximum likelihood classifiers
   * etc. The second type comprises statistic and neural clustering
   * methods.<p> All classifiers return an outputVector when classify
   * is called. The outputVector is created by the classifier by
   * applying its outputTemplate to the regular dvector which results
   * from the classification process.
   *
   */
  class classifier : public ioObject, public status {
  public:
    /**
     * constant to indicate an unknown object
     */
    static const int unknownObj;

    /**
     * constant to indicate "no object"
     */
    static const int noObject;

    /**
     * all probabilities under this value will be considered as 0
     */
    static const double nullProbability;

    /*
     * name of the unknown object
     */
//     static const std::string unknownName;

    /**
     * An output %vector is the result of classifying data with any
     * classifier.
     *
     * It is usually built by a classifier using its
     * internal outputTemplate. The outputVector contains labels (also
     * called ids) and values (usually probabilities). Typically, each
     * label occurs only once in the outputVector and all values are
     * between 0 and 1 and form a probability distribution. However,
     * this is not required by definition. Each classifier should
     * document the nature of its outputVector.
     *
     * In general, an ouputVector looks like this:
     * \image html outputVector0.png
     *
     * \image latex outputVector0.eps
     *
     * Some classifiers have multiple outputs for each class or
     * unnormalized values. In case this is not wanted in further
     * processing outputVector supplies several methods for
     * manipulation of labels and values.
     *
     * The method idMaximize() searches for all multiple ids and
     * erases all but the entry with the highest value. For the above
     * example this leads to the following outputVector:
     * \image html outputVectorLabelMax.png
     *
     * \image latex outputVectorLabelMax.eps
     *
     * The values for multiple ids are summed using the method
     * idSum(). After execution there is only one entry for each label
     * with the sum of values as new value. For the example the result
     * is:
     * \image html outputVectorLabelSum.png
     *
     * \image latex outputVectorLabelSum.eps
     *
     * Due to algorithmic reasons both methods sort the labels in
     * ascending order. You can check wether multiple labels exist with the
     * method noMultipleIds().
     *
     * For many application it is useful if the values of the
     * ouputVector form a probability distribution over the labels,
     * i.e. all values are greater equal zero and the sum over all
     * values is one. This is accomplished by the method
     * makeProbDistribution(). It sets values below zero to zero and
     * then normalizes the values to sum to one. When applied to the
     * two outputVectors above the method gives the following results:
     * \image html outputVectorLabelMaxProb.png
     *
     * \image html outputVectorLabelSumProb.png
     *
     * \image latex outputVectorLabelMaxProb.eps
     *
     * \image latex outputVectorLabelSumProb.eps
     *
     * A classifier or an independent module can decide that a
     * classification result does not contain any useful
     * informations. Such an outputVector is marked as rejected by
     * calling setReject() with argument either true or false. The
     * status is queried with isRejected().
     *
     * Some classifiers, instead of rejecting a result or not, give
     * confidence values. These lie between 0 for lowest and 1 for
     * highest confidence. The confidence is set with
     * setConfidenceValue() and queried with getConfidenceValue().
     *
     * \b NOTE: Confidence values are not taken into account when
     * algebraic methods are called.
     *
     * If possible all classifiers should define a winner unit. This
     * is the index (which usually corresponds to the same posisition
     * in an internal result) in the ouputVector which achieved the
     * best result in the classification. The winner unit is specified
     * by calling setWinnerUnit(). If the winner unit is the one with
     * the maximum value in the ouputVector, the method
     * setWinnerAtMax() can be used. The information is retrieved with
     * getWinnerUnit().
     *
     * If two or more outputVectors are to be merged or compared it is
     * useful to know whether they describe the same set of
     * labels. The method compatible() checks whether another
     * outputVector contains exactly the same labels as this
     * outputVector.
     *
     * Other functional groups of methods include:
     * - read-access methods: getId(), getValueByPosition(),
     *   getValueById(), getPair(), getValues(), getIds()
     * - write-access methods: setId(), setValueByPosition(),
     *   setValueById(), setPair(), setValues(), setIds(),setIdsAndValues()
     * - searchMethods: find(), maxValue(), maxPosition(), maxId(),
     *   maxPair(), minValue(), minPosition(), minId(),
     *   minPair()
     * - sorting methods: sortAscending(), sortDescending()
     * - algebraic methods: various methods that add or multiply
     *   ouputVectors or apply min or max operators.
     */
    class outputVector : public ioObject {
    public:

      /**
       * Default Constructor
       */
      outputVector();

      /**
       * Copy Constructor
       */
      outputVector(const outputVector& other);

      /**
       * Constructor. Creates an outputVector of the given size.
       * @param size size of the output vector
       */
      outputVector(const int& size);

      /**
       * Constructor. Sets the values and the ids.
       */
      outputVector(const ivector& theIds, const dvector& theValues);

      /**
       * copy method. Copy other into this and return this.
       */
      outputVector& copy(const outputVector& other);

      /**
       * copy operator. Copy other into this and return this.
       */
      outputVector& operator=(const outputVector& other);

      /**
       * clone method.
       */
      outputVector* clone() const;

      /**
       * Returns the number of elements in the outputVector
       */
      int size() const;

      /**
       * Finds the given id in the outputVector and returns its position if
       * contained in the %outputVector. If the id is not found false is
       * returned and the position set to -1.
       * @param id find this id
       * @param pos the position of the id.
       * @return true if id was found, false otherwise
       */
      bool find(const int& id, int& pos) const;

      /**
       * Sets the values of the outputVector.
       * The internal vector will be resized to the size of theValues.
       *
       * @param theValues the new values
       */
      void setValues(const dvector& theValues);

      /**
       * Sets the value at the given position of the outputVector.
       * @param pos vector entry, where the value should be written
       * @param value the new value
       * @return true if successful, false otherwise.
       */
      bool setValueByPosition(const int& pos, const double& value);

      /**
       * Sets the value for the given id in the outputVector.
       * @param id id whose value is to be changed
       * @param value the new value
       * @return true if successful, false otherwise.
       */
      bool setValueById(const int& id, const double& value);

      /**
       * Returns a const reference to the values of the %outputVector.
       */
      const dvector& getValues() const;

      /**
       * Sets the ids of the outputVector.
       * @param theIds the new ids
       */
      void setIds(const ivector& theIds);

      /**
       * Sets the ids of the outputVector and the values.
       *
       * The state of the instance will be exactly as after constructing an
       * outputVector with ids and values.
       *
       * If the sizes of both vectors are not equal, the smallest size will be
       * used and the largest vector will be cut.
       *
       * @param theIds the new ids
       * @param theValues the new values
       */
      void setIdsAndValues(const ivector& theIds,
                           const dvector& theValues);

      /**
       * Returns a const reference to the ids of the %outputVector
       */
      const ivector& getIds() const;

      /**
       * Sets the id and the value at the given position. If the position
       * does not exist, false is returned.
       * @param pos position to be changed
       * @param id id at that position
       * @param value value at that position
       * @return false on error
       */
      bool setPair(const int& pos, const int& id, const double& value);

      /**
       * Returns the id at the given position.
       * @param pos position in the vector
       * @param id the id at that position
       * @return true if id returned, false e.g. for invalid pos
       */
      bool getId(const int& pos, int& id) const;

      /**
       * Returns the value at the given position
       * @param pos position in the vector
       * @param value the value at that position
       * @return true if id returned, false e.g. for invalid pos
       */
      bool getValueByPosition(const int& pos, double& value) const;

      /**
       * Returns the value for the given id. If an id is used more than
       * once the value with the lowest position is returned.
       * @param id id value
       * @param value the value for the id
       * @return true if id returned, false e.g. for invalid pos
       */
      bool getValueById(const int& id, double& value) const;

      /**
       * Returns the id and the value at the given position.
       * @param pos position in the vector
       * @param id the id at that position
       * @param value the value at that position
       * @return true if id returned, false e.g. for invalid pos
       */
      bool getPair(const int& pos, int& id, double& value) const;

      /**
       * Set the winner unit
       * @param pos position of the winner unit in the vector
       * @return false if pos not between 0..n-1
       */
      bool setWinnerUnit(const int& pos);

      /**
       * Set the winner unit as the position of the maximum value of the
       * outputVector.
       * @return position of the winner unit in the vector
       */
      int setWinnerAtMax();

      /**
       * Get position of the winner
       */
      int getWinnerUnit() const;

      /**
       * Sets whether result is rejected. Default for reject is false.
       * @param rej true if rejected
       */
      void setReject(const bool& rej);

      /**
       * Returns whether the ouputVector was marked as rejected or not.
       */
      bool isRejected() const;

      /**
       * Sets the confidence value for this outputVector.
       * The value must be between 0 and 1 for lowest and highest confidence.
       * Default is 1.
       * Other values are cropped.
       * @param conf new confidence value betwenn 0 and 1
       */
      void setConfidenceValue(const double& conf);

      /**
       * Returns the confidence value for this outputVector. Values are
       * between zero (no confidence) and one (total confidence).
       */
      double getConfidenceValue() const;

      /**
       * Sorts the outputVector so that values are in ascending order.
       * The sort is only executed if the vector is not already sorted.
       * Ids and winner position are adjusted accordingly.
       */
      void sortAscending();

      /**
       * Sorts the outputVector so that values are in descending order.
       * The sort is only executed if the vector is not already sorted.
       * Ids and winner position are adjusted accordingly.
       */
      void sortDescending();

      /**
       * Lets the outputVector comply to rules for probability
       * distributions: Values between 0 and 1, sum of values equals
       * 1. This is done by setting values that are lower than zero to
       * zero and afterwards dividing each value by the sum of values.
       */
      void makeProbDistribution();

      /**
       * Checks whether the other outputVector contains exactly the same
       * ids as this outputVector.
       * @param other outputVector to be compared to this
       * @return true if ids are exactly the same.
       */
      bool compatible(const outputVector& other) const;

      /**
       * Returns true if there are never two elements of the outputVector
       * with the same id.
       */
      bool noMultipleIds() const;

      /**
       * If an id is given more than once, the maximum value is sought and
       * all other units of this id erased. The resulting outputVector is
       * sorted by ids.
       */
      void idMaximize();

      /**
       * If an id is given more than once, the sum of the values is
       * computed and only one unit of that id remains. The resulting
       * outputVector is sorted by ids.
       */
      void idSum();

      /**
       * Find the maximum value in the outputVector
       * @return the maximum value
       */
      double maxValue() const;

      /**
       * Find the position of the maximum value in the outputVector
       * @return the position of the maximum value
       */
      int maxPosition() const;

      /**
       * Find the id of the maximum value in the outputVector
       * @return the id of the maximum value
       */
      int maxId() const;

      /**
       * Find the id-value pair with the maximum value in the outputVector.
       * This is faster than finding them separately!
       * @param id id of the maximum value
       * @param value the maximum value
       */
      void maxPair(int& id, double& value) const;

      /**
       * Find the minimum value in the outputVector
       * @return the minimum value
       */
      double minValue() const;

      /**
       * Find the position of the minimum value in the outputVector
       * @return the position of the minimum value
       */
      int minPosition() const;

      /**
       * Find the id of the minimum value in the outputVector
       * @return the id of the minimum value
       */
      int minId() const;

      /**
       * Find the id-value pair with the minimum value in the outputVector.
       * This is faster than finding them separately!
       * @param id id of the minimum value
       * @param value the minimum value
       */
      void minPair(int& id, double& value) const;

      /**
       * Adds the other outputVector to this outputVector.
       * I.e. for each id that exists in both outputVectors the values
       * are added, else the value remains unchanged and the new ids
       * are appended to this outputVector.
       * @param other the outputVector to be added to this one
       * @return this outputVector
       */
      outputVector& add(const outputVector& other);

      /**
       * Adds the two outputVectors and writes the result into this
       * outputVector.
       * @param a the first summand
       * @param b the second summand
       * @return this outputVector
       * @see add(const outputVector& other)
       */
      outputVector& add(const outputVector& a, const outputVector& b);

      /**
       * Adds the given scalar to each value of the outputVector
       * @param s the scalar to be added
       * @return this outputVector
       */
      outputVector& add(const double& s);

      /**
       * Adds the other outputVector scaled by s to this outputVector.
       * I.e. for each id that exists in both outputVectors the values
       * are added, else the value remains unchanged and the new ids
       * are appended to this outputVector.
       * @param other the outputVector to be added to this one
       * @param s scaling factor for the other
       * @return this outputVector
       */
      outputVector& addScaled(const outputVector& other, const double& s);

      /**
       * Multiplies the other outputVector with this outputVector.
       * I.e. for each id that exists in both outputVectors the values
       * are multiplied, else the value is zero and the new ids
       * are appended to this outputVector.
       * @param other the outputVector to be multiplied to this one
       * @return this outputVector
       */
      outputVector& mul(const outputVector& other);

      /**
       * Multiplies the two outputVectors and writes the result into this
       * outputVector.
       * @param a the first factor
       * @param b the second factor
       * @return this outputVector
       * @see mul(const outputVector& other)
       */
      outputVector& mul(const outputVector& a, const outputVector& b);

      /**
       * Multiplies each value of the outputVector with the given scalar.
       * @param s the scalar
       * @return this outputVector
       */
      outputVector& mul(const double& s);

      /**
       * Divides each value of the outputVector by the given scalar.
       * @param s the scalar
       * @return this outputVector
       */
      outputVector& divide(const double& s);

      /**
       * Calculates the maximum between the other and this outputVector.
       * I.e. for each id that exists in both outputVectors the
       * maximum is taken, else the value stays unchanged and the new ids
       * are appended to this outputVector.
       * @param other the outputVector to be compared to this
       * @return this outputVector
       */
      outputVector& max(const outputVector& other);

      /**
       * Calculates the maximum between the two outputVectors and writes
       * it into this outputVector. Analog to inplace version.
       * @param a the first outputVector
       * @param b the second outputVector
       * @return this outputVector
       * @see max(const outputVector& other)
       */
      outputVector& max(const outputVector& a, const outputVector& b);

      /**
       * Calculates the minimum between the other and this outputVector.
       * I.e. for each id that exists in both outputVectors the
       * minimum is taken, else the value is zero and the new ids
       * are appended to this outputVector.
       * @param other the outputVector to be compared to this
       * @return this outputVector
       */
      outputVector& min(const outputVector& other);

      /**
       * Calculates the minimum between the two outputVectors and writes
       * it into this outputVector. Analog to inplace version.
       * @param a the first outputVector
       * @param b the second outputVector
       * @return this outputVector
       * @see min(const outputVector& other)
       */
      outputVector& min(const outputVector& a, const outputVector& b);

      /**
       * write the objectProb in the given ioHandler
       * @param handler the ioHandler to be used
       * @param complete if true (the default) the enclosing begin/end will
       *        be also written, otherwise only the data block will be written.
       * @return true if write was successful
       */
      bool write(ioHandler& handler,const bool complete=true) const;

      /**
       * read the objectProb from the given ioHandler
       * @param handler the ioHandler to be used
       * @param complete if true (the default) the enclosing begin/end will
       *        be also written, otherwise only the data block will be written.
       * @return true if write was successful
       */
      bool read(ioHandler& handler,const bool complete=true);

#     ifdef _LTI_MSC_6
      /**
       * this function is required by MSVC only, as a workaround for a
       * very awful bug, which exists since MSVC V.4.0, and still by
       * V.6.0 with all bugfixes (so called "service packs") remains
       * there...  This method is also public due to another bug, so please
       * NEVER EVER call this method directly: use read() instead
       */
      bool readMS(ioHandler& handler,const bool complete=true);

      /**
       * this function is required by MSVC only, as a workaround for a
       * very awful bug, which exists since MSVC V.4.0, and still by
       * V.6.0 with all bugfixes (so called "service packs") remains
       * there...  This method is also public due to another bug, so please
       * NEVER EVER call this method directly: use write() instead
       */
      bool writeMS(ioHandler& handler,const bool complete=true) const;
#     endif

    protected:

      /**
       * The ids of the the objects for which the corresponding values
       * (usually a probability) stand
       */
      ivector ids;

      /**
       * The actual results of each output unit. Usually, the values are
       * probabilities but other double values are legitimate as well.
       */
      dvector values;

      /**
       * The position in the outputVector that contains the winner
       * element, ie internal id.
       */
      int winner;

      /**
       * Expresses the confidence in this outputVector. A value of 0 means
       * no confidence, a value of 1 stands for 100% confidence.
       * Default is 1.
       */
      double confidence;

      /**
       * The outputVector is rejected when this flag is true
       */
      bool reject;

      /**
       * The outputVector is valid only when this flag is true (default).
       * Invalid results should not be used for further processing.
       */
      bool valid;

      /**
       * Indicates if vector is sorted in ascending order
       */
      bool sortedAsc;

      /**
       * Indicates if vector is sorted in descending order
       */
      bool sortedDesc;
    };

    /**
     * The %outputTemplate stores the relation between the different
     * positions (sometimes called internal ids) of a classification
     * result and the ids.  Applying the %outputTemplate to such a
     * %vector results in an outputVector which is not to be confused
     * with the classification result.
     *
     * There are two data structures within the outputTemplate storing the
     * relevant data:
     * - A simple list of ids, one for each element of the classification
     *   result. These are used, when the parameter multipleMode is set to
     *   Ignore. If %Ignore is set, but the data is not available, the
     *   %multipleMode is set to Max temporarily.
     * - For each element of the classification result exists a list of
     *   ids and respective probabilities. They state that, when that
     *   element is activated there is a certain probability that an
     *   input belonging to the class of the id was presented. These
     *   probabilities are usually generated by classifying a data-set and
     *   generating a probability distribution of the ids for the element
     *   of the classification result with the highest value. This data is
     *   used for all values of multipleMode but Ignore. If the data is not
     *   available, %multipleMode is set to %Ignore temprarily.
     *
     * The calculation of the outputVector using the apply method
     * depends on the value of the parameter multipleType, which is of
     * type eMultipleType. The following settings are available:
     *
     * <dl>
     * <dt>Ignore</dt>
     * <dd>
     *  If default ids have been stored in the %outputTemplate via the
     *  constructor that receives an %ivector, the method setIds or setData
     *  these ids are simply copied to the %outputVector. I.e. no statistics
     *  about the actual classification performance of the %classifier are
     *  used. If the data is not set, the option Max is used and false is
     *  returned by the apply method.
     * </dd>
     * <dt>Max</dt>
     * <dd>
     *  The probability lists are used. For each element of the classification
     *  result, the id with the highest probability is found and set to one
     *  while all other probabilities for that element are set to zero. This
     *  leads to an %outputVector which is equal or similar to the one
     *  generated by Ignore. There will be differences, however, if a certain
     *  element of the classification result was trained for one class, but
     *  when building the probability distributions another class caused this
     *  element to have the highest value more frequently. This case can be
     *  seen for the second element in the example below.
     * </dd>
     * <dt>Uniform</dt>
     * <dd>
     *  The probability lists are used. For each of the classification result
     *  the number of ids in the list is found and their probabilities set
     *  to be uniformly distributed. This method puts very little trust in
     *  the data used for generating the probabilities, i.e. that it
     *  represents the true distribution of the data. On the other hand,
     *  it is very susceptible to noise in the data: One misclassified
     *  example can completely change the outcome of future classifications.
     * </dd>
     * <dt>ObjProb</dt>
     * <dd>
     *  The probablity lists are used. The complete information is used.
     *  This has a functionality similar to a rule set: If element A is
     *  activated, then there is a probability of 0.3 for class 1 and 0.7
     *  for class 5. This method works quite well, when the training data
     *  represents the actual distributions quite well, but the classifier
     *  is not able to build the correct models. A typical effect of using
     *  this approach rather than Ignore is that misclassified unknown data
     *  will have greater probability and thus a higher ranking. On the
     *  downside, sometimes data correctly classified by Ignore can be
     *  just misclassified.
     * </dd>
     * </dl>
     *
     * As mentioned above for all cases but Ignore, the
     * %outputTemplate contains a list of class probabilities for each
     * element of the classification result. These are interpreted as
     * dependent probablities: P(o|x) where o stands for
     * the id and x for the position in the classification
     * result. Each element of the classification results is also
     * taken as a probability p(x). Thus the values for each id
     * are calculated as \f$P(o)=\sum_x p(x)\cdot P(o|x)\f$. <br>
     *
     * Here is a short example for the behavior of an outputTemplate
     * when applied to a classification result. The figure shows the
     * classification result on the lefthand side, the default ids
     * which are used with the option Ignore in the middle and the
     * probabiltity lists which are used for Max, Uniform and ObjProb
     * on the righthand side.
     *
     * \image html outputTemplate.png
     *
     * \image latex outputTemplate.eps
     *
     * Depending on the value of multipleMode the following outputVector is
     * generated by calling apply:
     *
     * <table border=1>
     * <tr>
     *  <td> </td><td>1</td><td>3</td><td>5</td>
     *  <td>6</td><td>17</td><td>22</td><td>41</td>
     * </tr><tr>
     *  <td>Ignore</td><td>0.15</td><td>0.50</td><td>---</td>
     *  <td>---</td><td>0.03</td><td>0.30</td><td>0.02</td>
     * </tr><tr>
     *  <td>Max</td><td>0.15</td><td>---</td><td>---</td>
     *  <td>0.50</td><td>0.03</td><td>0.30</td><td>0.02</td>
     * </tr><tr>
     *  <td>Uniform</td><td>0.15</td><td>0.35</td><td>0.10</td>
     *  <td>0.25</td><td>0.04</td><td>0.10</td><td>0.01</td>
     * </tr><tr>
     *  <td>ObjProb</td><td>0.15</td><td>0.33</td><td>0.05</td>
     *  <td>0.27</td><td>0.04</td><td>0.15</td><td>0.01</td>
     * </tr>
     * </table>
     *
     * If the use of all four options is desired, the constructor
     * outputTemplate(int) receiving an int value must be used. All
     * data can be set using methods setIds, setProbs and/or
     * setData. If the other constructors are used, no space is
     * reserved for the lists of probabilities, since these take much
     * space and some, especially unsupervised, %classifiers do not
     * need or have no means to gather this information.
     */
    class outputTemplate : public ioObject {

    public:

      /**
       * This type specifies how the output element probability and
       * the probabilities in the list should be combined. See description of
       * outputTemplate.
       */
      enum eMultipleMode{
        Ignore = 0, /*!< ignore the %object probability */
        Max,        /*!< set the prob of the id with max prob to 1, others
                      to zero. */
        Uniform,    /*!< assume that all objects in the list of one output
                         element have the same probability
                         (1/number of elements).  */
        ObjProb     /*!< consider the given %object probabilities */
      };


      /**
       * Default constructor. <p>
       * multipleMode is ObjProb.
       */
      outputTemplate();

      /**
       * Copy constructor.
       */
      outputTemplate(const outputTemplate& other);

      /**
       * Constructor. Since a vector of ids is given multipleMode is Ignore and
       * the probability lists are not initialized and thus cannot be set
       * later.
       */
      outputTemplate(const ivector& theIds);

      /**
       * Constructor. The number of output units is
       * given. multipleMode is ObjeProb. Default ids as well as lists
       * of probabilities can be set.
       */
      outputTemplate(const int& size);

      /**
       * copy
       */
      outputTemplate& copy(const outputTemplate& other);

      /**
       * assigment operator (alias for copy(other)).
       * @param other the outputTemplate to be copied
       * @return a reference to the actual outputTemplate
       */
      inline outputTemplate& operator=(const outputTemplate& other) {
        return copy(other);
      }

      /**
       * clone
       */
      outputTemplate* clone() const;

      /**
       * Change the setting of how the object probabilities of each unit
       * are taken into account when calculating the outputVector. See
       * description of outputTemplate.
       */
      void setMultipleMode(const eMultipleMode& mode);

      /**
       * Get the setting of multipleMode
       */
      eMultipleMode getMultipleMode() const;

      /**
       * Set the default id vector. These are used when multipleMode is set
       * to Ignore.
       */
      bool setIds(const ivector& theIds);

      /**
       * Returns a const reference to the id vector.
       */
      const ivector& getIds() const;

      /**
       * Set the probabilities of one unit. This information must be set for
       * all elements of the classification result. Then is can be used
       * by the apply method when multipleMode is set to one of Max, Uniform
       * or ObjProb.
       * @param pos the posision in the classification result this distribution
       *            is for-
       * @param theIds list of ids of classes possibly correct, when this
       *               position has high probability.
       * @param theValues probabilities of each of these ids.
       * @return false on errer, e.g. illegal pos
       */
      bool setProbs(const int& pos, const ivector& theIds,
                    const dvector& theValues);

      /**
       * Set the probabilities of one unit. This information must be set for
       * all elements of the classification result. Then is can be used
       * by the apply method when multipleMode is set to one of Max, Uniform
       * or ObjProb.
       * @param pos the posision in the classification result this distribution
       *            is for-
       * @param outV list of ids and corresponding probabilities of classes
       *             possibly correct, when this position has high
       *             probability.
       * @return false on error, e.g. illegal pos
       */
      bool setProbs(const int& pos, const outputVector& outV);

      /**
       * Returns a const reference to the probability distribution at
       * the given position of the template.
       */
      const outputVector& getProbs(const int& pos) const;

      /**
       * Set the probabilities and the default id of one unit. This
       * information must be set for all elements of the
       * classification result. Then is can be used by the apply
       * method for any value of multipleMode.
       * @param pos the posision in the classification result this distribution
       *            is for.
       * @param realId the expected or desired id of this posision of the
       *               classification result.
       * @param outV list of ids and corresponding probabilities of classes
       *              possibly correct, when this position has high
       *              probability.
       * @return false on error, e.g. illegal pos
       */
      bool setData(const int& pos, const int& realId,
                   const outputVector& outV);

      /**
       * returns the number of output units handled by this outputTemplate
       */
      int size() const;

      /**
       * Uses the information stored in the outputTemplate to generate
       * an outputVector from a dvector. See description of
       * outputTemplate for details. The classification result should
       * contain only positive values which are greater for better
       * fit. The best interpretability is obtained if data is a
       * probability distribution.
       * @param data the classification result
       * @param result %outputVector calculted using the outputTemplate.
       * @return false on error (check getStatusString())
       */
      bool apply(const dvector& data, outputVector& result) const;

      /**
       * write the outputTemplate in the given ioHandler
       * @param handler the ioHandler to be used
       * @param complete if true (the default) the enclosing begin/end will
       *        be also written, otherwise only the data block will be written.
       * @return true if write was successful
       */
      bool write(ioHandler& handler,const bool complete=true) const;

      /**
       * read the outputTemplate from the given ioHandler
       * @param handler the ioHandler to be used
       * @param complete if true (the default) the enclosing begin/end will
       *        be also written, otherwise only the data block will be written.
       * @return true if write was successful
       */
      bool read(ioHandler& handler,const bool complete=true);

#     ifdef _LTI_MSC_6
      /**
       * this function is required by MSVC only, as a workaround for a
       * very awful bug, which exists since MSVC V.4.0, and still by
       * V.6.0 with all bugfixes (so called "service packs") remains
       * there...  This method is also public due to another bug, so please
       * NEVER EVER call this method directly: use read() instead
       */
      bool readMS(ioHandler& handler,const bool complete=true);

      /**
       * this function is required by MSVC only, as a workaround for a
       * very awful bug, which exists since MSVC V.4.0, and still by
       * V.6.0 with all bugfixes (so called "service packs") remains
       * there...  This method is also public due to another bug, so please
       * NEVER EVER call this method directly: use write() instead
       */
      bool writeMS(ioHandler& handler,const bool complete=true) const;
#     endif

    protected:


      /**
       * Determines what data is used for calculation of an
       * outputVector from the classification result and the
       * outputTemplate.  See eUseObjectProb and outputTemplate for
       * detailed description.
       */
      eMultipleMode multipleMode;

      /**
       * Contains one outputVector for each output unit. These
       * hold the probabilities for the ids being correct when this
       * unit is activated.
       */
      std::vector<outputVector> probList;

      /**
       * List of ids for each output unit.
       */
      ivector defaultIds;

    };



    // --------------------------------------------------
    // classifier::parameters
    // --------------------------------------------------

    /**
     * the parameters for the class classifier
     */
    class parameters : public ioObject {
    public:

      /**
       * indicates the distance measure used for the classifier
       */
      enum eDistanceMeasure {
        L1, /*!< L1-norm (sum of the absolut values) */
        L2  /*!< L2-norm (square root of the sum of the squares)*/
      };


      /**
       * Sets the mode the outputTemplate of the %classifier works in,
       * if it uses multipleIds. This option is usually fixed for all
       * %classifiers. unsupervised %classifiers methods usually wont
       * use multiple ids, whereas supervised %classifiers will. The
       * default is ObjProb.<p>
       * Setting this parameters only makes sense before using classify,
       * since all information is recorded during training anyway.
       */
      outputTemplate::eMultipleMode multipleMode;

      /**
       * default constructor
       */
      parameters();

      /**
       * copy constructor
       * @param other the parameters %object to be copied
       */
      parameters(const parameters& other);

      /**
       * destructor
       */
      virtual ~parameters();

      /**
       * returns name of this type
       */
      const char* getTypeName() const;

      /**
       * copy the contents of a parameters %object
       * @param other the parameters %object to be copied
       * @return a reference to this parameters %object
       */
      parameters& copy(const parameters& other);

      /**
       * Alias for copy
       */
      inline parameters& operator=(const parameters& other) {
        return copy(other);
      }


      /**
       * returns a pointer to a clone of the parameters
       */
      virtual parameters* clone() const;

      /**
       * write the parameters in the given ioHandler
       * @param handler the ioHandler to be used
       * @param complete if true (the default) the enclosing begin/end will
       *        be also written, otherwise only the data block will be written.
       * @return true if write was successful
       */
      virtual bool write(ioHandler& handler,const bool complete=true) const;

      /**
       * read the parameters from the given ioHandler
       * @param handler the ioHandler to be used
       * @param complete if true (the default) the enclosing begin/end will
       *        be also written, otherwise only the data block will be written.
       * @return true if write was successful
       */
      virtual bool read(ioHandler& handler,const bool complete=true);

#     ifdef _LTI_MSC_6
      /**
       * this function is required by MSVC only, as a workaround for a
       * very awful bug, which exists since MSVC V.4.0, and still by
       * V.6.0 with all bugfixes (so called "service packs") remains
       * there...  This method is also public due to another bug, so please
       * NEVER EVER call this method directly: use read() instead
       */
      bool readMS(ioHandler& handler,const bool complete=true);

      /**
       * this function is required by MSVC only, as a workaround for a
       * very awful bug, which exists since MSVC V.4.0, and still by
       * V.6.0 with all bugfixes (so called "service packs") remains
       * there...  This method is also public due to another bug, so please
       * NEVER EVER call this method directly: use write() instead
       */
      bool writeMS(ioHandler& handler,const bool complete=true) const;
#     endif

    };

    // --------------------------------------------------
    // exceptions
    // --------------------------------------------------

    /**
     * Exception thrown when the output %objects has not been set yet
     */
    class invalidOutputException : public exception {
    public:
      /**
       * Default constructor
       */
      invalidOutputException()
        : exception("wrong classifier output type or output not set yet") {};

      /**
       * returns the name of this exception
       */
      virtual const char* getTypeName() const {
        return "classifier::invalidOutputException";
      };
    };

    /**
     * Exception thrown when the parameters are not set
     */
    class invalidParametersException : public exception {
    public:
      /**
       * Default constructor
       */
      invalidParametersException()
        : exception("wrong parameter type or parameters not set yet") {};

      /**
       * Constructor with the alternative object name, where the exception
       * was thrown
       */
      invalidParametersException(const std::string& str)
        : exception(std::string("wrong parameter type or parameters not set yet at ")+str) {};

      /**
       * Constructor with the alternative object name, where the exception
       * was thrown
       */
      invalidParametersException(const char* str)
        : exception(std::string("wrong parameter type or parameters not set yet at ")+std::string(str)) {};

      /**
       * returns the name of this exception
       */
      virtual const char* getTypeName() const;
    };

    /**
     * Exception thrown when a method of a functor is not implemented for
     * a specific parameter set.
     */
    class invalidMethodException : public exception {
    public:
      /**
       * Default constructor
       */
      invalidMethodException()
        : exception("Method not implemented for given parameters") {};
      /**
       * returns the name of this exception
       */
      virtual const char* getTypeName() const;
    };

    /**
     * Exception thrown when a parameter is out of range
     */
    class parametersOutOfRangeException : public exception {
    public:
      /**
       * Default constructor
       */
      parametersOutOfRangeException()
        : exception("at least one parameter is out range") {};

      /**
       * returns the name of this exception
       */
      virtual const char* getTypeName() const;
    };

    // --------------------------------------------------
    // classifier
    // --------------------------------------------------

    /**
     * default constructor
     */
    classifier();

    /**
     * copy constructor
     * @param other the %object to be copied
     */
    classifier(const classifier& other);

    /**
     * destructor
     */
    virtual ~classifier();

    /**
     * returns the name of this type ("classifier")
     */
    virtual const char* getTypeName() const;

//     /**
//      * return the last message set with setStatusString().  This will
//      * never return 0.  If no status-string has been set yet an empty string
//      * (pointer to a string with only the char(0)) will be returned.
//      */
//     virtual const char* getStatusString() const;

//     /**
//      * set a status string.
//      *
//      * @param msg the const string to be reported next time by
//      * getStatusString().  The given string will be copied
//      * This message will be usually set within the apply methods to indicate
//      * an error cause.
//      *
//      * Note that the change of the status string is not considered as
//      * a change in the functor status.
//      */
//     virtual void setStatusString(const char* msg) const;

    /**
     * copy data of "other" functor.
     * @param other the functor to be copied
     * @return a reference to this functor %object
     */
    classifier& copy(const classifier& other);

    /**
     *
     * Alias for "copy".
     */
    inline classifier& operator=(const classifier& other) {
      return copy(other);
    }

    /**
     * returns a pointer to a clone of this functor.
     */
    virtual classifier* clone() const = 0;

    /**
     * returns true if the parameters are valid
     */
    virtual bool validParameters() const {return (params != 0);};

    /**
     * returns used parameters
     */
    const parameters& getParameters() const;

    /**
     * sets the classifier's parameters.
     * This member makes a copy of <em>theParam</em>: the classifier
     * will keep its own copy of the parameters!
     */
    virtual bool setParameters(const parameters& theParam);

    /**
     * @name Progress Info
     *
     * Methods used to plug-in and retrieve progress visualization objects.
     */
    //@{
    
    /**
     * set a progress %object
     *
     * A clone of the given %object will be generated.
     */
    void setProgressObject(const progressInfo& progBox);

    /**
     * remove the active progress %object
     */
    void removeProgressObject();

    /**
     * return true if a valid progressInfo %object has already been setted
     */
    bool validProgressObject() const;

    /**
     * get progress %object
     */
    progressInfo& getProgressObject();

    /**
     * get progress %object
     */
    const progressInfo& getProgressObject() const;
    //@}

    /**
     * sets the classifier's outputTemplate
     * This member makes a copy of <em>theOutputTemplate</em>: the classifier
     * will keep its own copy of the outputTemplate
     */
    virtual void setOutputTemplate(const outputTemplate& theOutputTemplate);

    /**
     * get a const reference to the outputTemplate
     */
    const outputTemplate& getOutputTemplate() const;

    /**
     * write the classifier in the given ioHandler
     * @param handler the ioHandler to be used
     * @param complete if true (the default) the enclosing begin/end will
     *        be also written, otherwise only the data block will be written.
     * @return true if write was successful
     */
    virtual bool write(ioHandler& handler,const bool complete=true) const;

    /**
     * read the classifier from the given ioHandler
     * @param handler the ioHandler to be used
     * @param complete if true (the default) the enclosing begin/end will
     *        be also written, otherwise only the data block will be written.
     * @return true if write was successful
     */
    virtual bool read(ioHandler& handler,const bool complete=true);

#ifndef SWIG
  typedef parameters classifier_parameters;
  typedef outputVector classifier_outputVector;
  typedef outputTemplate classifier_outputTemplate;
#endif
  
  protected:
    /**
     * returns current parameters. (non const! -> protected)
     */
    //parameters& getParameters() {return *params;};

    /**
     * current parameters.
     */
    parameters* params;

    /**
     * current progress %object
     */
    progressInfo* progressBox;

//     /**
//      * the status string written with setStatusString
//      */
//     mutable char* statusString;

//     /**
//      * the empty string returned if the statusString is empty
//      */
//     static const char *const emptyString;

    /**
     * The outputTemplate for this classifier
     */
    outputTemplate outTemplate;

  };


  /**
   * write the functor::parameters in the given ioHandler.
   * The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   */
  bool write(ioHandler& handler,const classifier::parameters& p,
             const bool complete = true);

  /**
   * read the functor::parameters from the given ioHandler.
   * The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   */
  bool read(ioHandler& handler,classifier::parameters& p,
             const bool complete = true);

  /**
   * write the classifier in the given ioHandler.
   * The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   */
  bool write(ioHandler& handler,const classifier& p,
             const bool complete = true);

  /**
   * read the classifier from the given ioHandler.
   * The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   */
  bool read(ioHandler& handler,classifier& p,
             const bool complete = true);

  /**
   * write the functor::parameters in the given ioHandler.
   * The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   */
  bool write(ioHandler& handler,const classifier::outputVector& p,
             const bool complete = true);

  /**
   * read the functor::parameters from the given ioHandler.
   * The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   */
  bool read(ioHandler& handler,classifier::outputVector& p,
             const bool complete = true);

  /**
   * write the functor::parameters in the given ioHandler.
   * The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   */
  bool write(ioHandler& handler,const classifier::outputTemplate& p,
             const bool complete = true);

  /**
   * read the functor::parameters from the given ioHandler.
   * The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   */
  bool read(ioHandler& handler,classifier::outputTemplate& p,
             const bool complete = true);

}

namespace std {
  ostream&
  operator<<(ostream& s,const lti::classifier::outputVector& o);
}

#endif

