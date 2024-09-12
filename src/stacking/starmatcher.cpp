#include <astrophoto-toolbox/stacking/starmatcher.h>
#include <astrophoto-toolbox/algorithms/math.h>
#include <Eigen/Core>
#include <Eigen/LU>

using namespace astrophototoolbox;
using namespace stacking;


bool StarMatcher::computeTransformation(
    const star_list_t& fromStars, const star_list_t& toStars, const size2d_t& imageSize,
    Transformation& transformation
)
{
	if ((toStars.size() <= 4) || ((toStars.size() < fromStars.size() / 5) && (toStars.size() < 30)))
        return false;

    this->imageSize = imageSize;

    star_list_t sortedFromStars = fromStars;
    star_list_t sortedToStars = toStars;

	std::sort(sortedFromStars.begin(), sortedFromStars.end(), star_t::compareIntensity);
	std::sort(sortedToStars.begin(), sortedToStars.end(), star_t::compareIntensity);

    references.clear();
    targets.clear();

    for (size_t i = 0; i < std::min(sortedFromStars.size(), size_t(100)); ++i)
        references.emplace_back(sortedFromStars[i].position);

    for (size_t i = 0; i < std::min(sortedToStars.size(), size_t(100)); ++i)
        targets.emplace_back(sortedToStars[i].position);

    bool result = false;

    if ((references.size() >= 8) && (targets.size() >= 8))
        result = computeLargeTriangleTransformation(transformation);

    return result;
}

//-----------------------------------------------------------------------------

std::vector<std::tuple<point_t, point_t>> StarMatcher::pairs() const
{
    std::vector<std::tuple<point_t, point_t>> pairs;

    for (const auto& pair : votedPairs)
    {
        const auto& ref = references[pair.refStar];
        const auto& target = targets[pair.targetStar];

        pairs.push_back({ point_t(ref.x, ref.y), point_t(target.x, target.y) });
    }

    return pairs;
}

//-----------------------------------------------------------------------------

bool StarMatcher::computeLargeTriangleTransformation(Transformation& transforms)
{
    bool result = false;

    // Compute the distances between the stars
    computeStarDistances(references, referenceDistances);
    for (int i = 0; i < referenceDistances.size(); ++i)
        referenceIndices.push_back(i);

    computeStarDistances(targets, targetDistances);
    for (int i = 0; i < targetDistances.size(); ++i)
        targetIndices.push_back(i);

    std::sort(
        referenceIndices.begin(), referenceIndices.end(),
        [this](int dist1, int dist2) {
           return referenceDistances[dist1].distance > referenceDistances[dist2].distance;
        }
    );

    std::sort(
        targetIndices.begin(), targetIndices.end(),
        [this](int dist1, int dist2) {
           return targetDistances[dist1].distance > targetDistances[dist2].distance;
        }
    );

    // Pair stars from both images
    voting_pair_list_t votingPairs;

    initVotingGrid(votingPairs);

    auto addVote = [](uint8_t refStar, uint8_t targetStar, voting_pair_list_t& votingPairs, size_t nbTargetStars)
    {
        int offset = refStar * nbTargetStars + targetStar;
        votingPairs[offset].nbVotes++;
    };

    int i = 0;
    int j = 0;
    while ((i < targetDistances.size()) && (j < referenceDistances.size()))
    {
        if (fabs(targetDistances[targetIndices[i]].distance - referenceDistances[referenceIndices[j]].distance) <= MAXSTARDISTANCEDELTA)
        {
            // These are within 2 pixels ... find all the others stars using the same stars
            // in the targets list and check if the distances are the same in the reference.
            // If it is the case, cast a vote for each potential pair.
            float refDistance12 = referenceDistances[referenceIndices[j]].distance;
            float targetDistance12 = targetDistances[targetIndices[i]].distance;

            int refStar1 = referenceDistances[referenceIndices[j]].star1;
            int refStar2 = referenceDistances[referenceIndices[j]].star2;

            int targetStar1 = targetDistances[targetIndices[i]].star1;
            int targetStar2 = targetDistances[targetIndices[i]].star2;

            for (int targetStar3 = 0; targetStar3 < targets.size(); ++targetStar3)
            {
                if ((targetStar3 == targetStar1) || (targetStar3 == targetStar2))
                    continue;

                stars_distance_list_t::iterator it;
                double targetDistance13 = 0.0;
                double targetDistance23 = 0.0;
                double ratio;

                it = std::lower_bound(targetDistances.begin(), targetDistances.end(), StarsDistance(targetStar1, targetStar3));
                if (it != targetDistances.end())
                    targetDistance13 = (*it).distance;

                it = std::lower_bound(targetDistances.begin(), targetDistances.end(), StarsDistance(targetStar2, targetStar3));
                if (it != targetDistances.end())
                    targetDistance23 = (*it).distance;

                ratio = std::max(targetDistance13, targetDistance23) / targetDistance12;

                // 0.9 avoids many useless triangles with two big sides and one small side
                if (ratio < 0.9)
                {
                    // Search a star from reference such as
                    //   - distance from 1 to 3 is near targetDistance13
                    //   - distance from 2 to 3 is near targetDistance23
                    for (int refStar3 = 0; refStar3 < references.size(); ++refStar3)
                    {
                        if ((refStar3 == refStar1) || (refStar3 == refStar2))
                            continue;

                        double refDistance13 = 0.0;
                        double refDistance23 = 0.0;

                        it = std::lower_bound(referenceDistances.begin(), referenceDistances.end(), StarsDistance(refStar1, refStar3));
                        if (it != referenceDistances.end())
                            refDistance13 = (*it).distance;

                        it = std::lower_bound(referenceDistances.begin(), referenceDistances.end(), StarsDistance(refStar2, refStar3));
                        if (it != referenceDistances.end())
                            refDistance23 = (*it).distance;

                        if ((fabs(refDistance13 - targetDistance13) < MAXSTARDISTANCEDELTA) &&
                            (fabs(refDistance23 - targetDistance23) < MAXSTARDISTANCEDELTA))
                        {
                            // Cast votes for stars
                            addVote(refStar1, targetStar1, votingPairs, (int) targets.size());
                            addVote(refStar2, targetStar2, votingPairs, (int) targets.size());
                            addVote(refStar3, targetStar3, votingPairs, (int) targets.size());
                        }
                        else if ((fabs(refDistance23 - targetDistance13) < MAXSTARDISTANCEDELTA) &&
                                 (fabs(refDistance13 - targetDistance23) < MAXSTARDISTANCEDELTA))
                        {
                            // Cast votes for stars
                            addVote(refStar1, targetStar2, votingPairs, (int) targets.size());
                            addVote(refStar2, targetStar1, votingPairs, (int) targets.size());
                            addVote(refStar3, targetStar3, votingPairs, (int) targets.size());
                        }
                    }
                }
            }
        }

        if (targetDistances[targetIndices[i]].distance < referenceDistances[referenceIndices[j]].distance)
            j++;
        else
            i++;
    }

    // Sort voting pairs in descending order
    std::sort(
        votingPairs.begin(), votingPairs.end(),
        [](const VotingPair& a, const VotingPair& b) { return a.nbVotes > b.nbVotes; }
    );

    // Eliminate false matches and get transformations parameters
    if (!votingPairs.empty())
    {
        int minNbVotes;
        int cut = 0;

        minNbVotes = std::max(votingPairs[targets.size() * 2 - 1].nbVotes, (unsigned) 1);

        while (votingPairs[cut].nbVotes >= minNbVotes)
            ++cut;

        votingPairs.resize(cut+1);

        result = computeSigmaClippingTransformation(votingPairs, transforms);
    }

    return result;
}

//-----------------------------------------------------------------------------

void StarMatcher::computeStarDistances(const point_list_t& stars, stars_distance_list_t& distances)
{
    double maxDistance = 0;

    distances.clear();
    distances.reserve(stars.size() * (stars.size() - 1) / 2);

    for (int i = 0; i < stars.size(); ++i)
    {
        for (int j = i + 1; j < stars.size(); ++j)
        {
            double distance = stars[i].distance(stars[j]);
            maxDistance = std::max(distance, maxDistance);
            distances.emplace_back(i, j, distance);
        };
    };

    std::sort(distances.begin(), distances.end());
}

//-----------------------------------------------------------------------------

void StarMatcher::initVotingGrid(voting_pair_list_t& votingPairs)
{
    votingPairs.clear();
    votingPairs.reserve(references.size() * targets.size());

    for (int i = 0; i < references.size(); ++i)
    {
        for (int j = 0; j < targets.size(); ++j)
            votingPairs.push_back(VotingPair(i, j));
    }
}

//-----------------------------------------------------------------------------

bool StarMatcher::computeSigmaClippingTransformation(
    const voting_pair_list_t& votingPairs, Transformation& transforms
)
{
    bool result = false;
    voting_pair_list_t pairs = votingPairs;

    //Transformation baseTransformation;

    // First try to compute a robust bilinear transformation
    result = computeCoordinatesTransformation(pairs, transforms);
    if (!result)
        return false;

    // Remove inactive pairs from the resulting pairs
    votedPairs.clear();
    for (int i = 0; i < pairs.size(); ++i)
    {
        if (pairs[i].isActive())
            votedPairs.push_back(pairs[i]);
    }

    return result;
}

//-----------------------------------------------------------------------------

bool StarMatcher::computeCoordinatesTransformation(
    voting_pair_list_t& votingPairs, Transformation& transforms
)
{
    bool result = false;
    voting_pair_list_t pairs;

    const size_t nbPairs = 8;
    std::vector<int> addedPairs;
    voting_pair_list_t testedPairs;
    voting_pair_list_t goodPairs;
    std::vector<int> goodAddedPairs;
    Transformation goodTransformation;

    pairs = votingPairs;

    while (!result)
    {
        addedPairs.clear();
        testedPairs.clear();

        // Add active pairs up to the limit
        for (size_t i = 0; (i < pairs.size()) && (testedPairs.size() < nbPairs); ++i)
        {
            if (pairs[i].isActive())
            {
                testedPairs.push_back(pairs[i]);
                addedPairs.push_back(static_cast<int>(i));
            }
        }

        // Not enough stars left: stop here
        if (testedPairs.size() != nbPairs)
            break;

        // Compute the transformation
        Transformation transformation;

        if (computeTransformation(testedPairs, transformation))
        {
            std::vector<double> distances;
            double maxDistance = 0.0;
            size_t maxDistanceIndex = 0;

            // Compute the distance between the stars and their projection
            for (size_t i = 0; i < testedPairs.size(); i++)
            {
                const point_t projected = transformation.transform(targets[testedPairs[i].targetStar]);
                const double distance = projected.distance(references[testedPairs[i].refStar]);

                distances.push_back(distance);
                if (distance > maxDistance)
                {
                    maxDistance = distance;
                    maxDistanceIndex = i;
                }
            }

            // If one star is far from the spot - deactivate the pair
            if (maxDistance > 3)
            {
                int deactivatedIndice;
                double average;
                double sigma;
                bool oneDeactivated = false;

                sigma = computeStandardDeviation(distances, average);

                for (size_t i = 0; i < distances.size(); ++i)
                {
                    if (fabs(distances[i] - average) > 2 * sigma)
                    {
                        deactivatedIndice = addedPairs[i];
                        pairs[deactivatedIndice].setActive(false);
                        oneDeactivated = true;
                    }
                }

                if (!oneDeactivated)
                {
                    for (size_t i = 0; i < distances.size(); ++i)
                    {
                        if (fabs(distances[i] - average) > sigma)
                        {
                            deactivatedIndice = addedPairs[i];
                            pairs[deactivatedIndice].setActive(false);
                            oneDeactivated = true;
                        }
                    }
                }

                if (!oneDeactivated)
                {
                    deactivatedIndice = addedPairs[maxDistanceIndex];
                    pairs[deactivatedIndice].setActive(false);
                }
            }
            else
            {
                goodTransformation = transformation;
                goodPairs = testedPairs;
                goodAddedPairs = addedPairs;
                result = true;
            }
        }
        else
        {
            // Remove the last pair of the selected pairs
            pairs[addedPairs[nbPairs - 1]].setActive(false);
        }
    }

    if (goodPairs.empty())
        return false;

    // Try to add other pairs to refine the transformation
    Transformation transformation = goodTransformation;
    voting_pair_list_t tempPairs;
    int nbFails = 0;

    testedPairs = goodPairs;
    addedPairs = goodAddedPairs;

    for (size_t index : addedPairs)
        votingPairs[index].setUsed(true);

    while (true)
    {
        double maxDistance;
        bool transformOk = false;
        int addedPair = -1;

        tempPairs = testedPairs;
        for (size_t i = 0; (i < votingPairs.size()) && (addedPair < 0); ++i)
        {
            if (votingPairs[i].isActive() && !votingPairs[i].isUsed())
            {
                addedPair = static_cast<int>(i);
                tempPairs.push_back(votingPairs[i]);
                votingPairs[addedPair].setUsed(true);
            }
        }

        if (addedPair < 0)
            break;

        if (computeTransformation(tempPairs, transformation))
        {
            maxDistance = validateTransformation(tempPairs, transformation);
            if (maxDistance <= 2)
            {
                testedPairs = tempPairs;
                transforms = transformation;
                addedPairs.push_back(addedPair);
                transformOk = true;
            }
            else
                votingPairs[addedPair].setActive(false);
        };

        if (!transformOk)
        {
            ++nbFails;
            if (nbFails > 3)
                break;
        }
    }

    votingPairs = testedPairs;

    return true;
}

//-----------------------------------------------------------------------------

bool StarMatcher::computeTransformation(
    const voting_pair_list_t & votingPairs, Transformation& transforms
)
{
    transforms.xWidth = imageSize.width;
    transforms.yWidth = imageSize.height;

    Eigen::MatrixXd M(votingPairs.size(), 4);
    Eigen::MatrixXd X(votingPairs.size(), 1);
    Eigen::MatrixXd Y(votingPairs.size(), 1);

    for (int i = 0; i < votingPairs.size(); ++i)
    {
        point_t& star = references[votingPairs[i].refStar];
        X(i, 0) = star.x / transforms.xWidth;
        Y(i, 0) = star.y / transforms.yWidth;
    }

    for (int i = 0; i < votingPairs.size(); ++i)
    {
        point_t& star = targets[votingPairs[i].targetStar];
        double X = star.x / transforms.xWidth;
        double Y = star.y / transforms.yWidth;

        M(i, 0) = 1.0;
        M(i, 1) = X;
        M(i, 2) = Y;
        M(i, 3) = X * Y;
    };

    Eigen::MatrixXd MT = M.transpose();
    Eigen::MatrixXd TM = MT * M;

    Eigen::FullPivLU<Eigen::MatrixXd> lu(TM);
    if (!lu.isInvertible())
        return false;

    Eigen::MatrixXd A = TM.inverse() * MT * X;
    Eigen::MatrixXd B = TM.inverse() * MT * Y;

    transforms.a0 = A(0, 0);
    transforms.a1 = A(1, 0);
    transforms.a2 = A(2, 0);
    transforms.a3 = A(3, 0);
    transforms.b0 = B(0, 0);
    transforms.b1 = B(1, 0);
    transforms.b2 = B(2, 0);
    transforms.b3 = B(3, 0);

    return true;



    // typedef math::matrix<double> DMATRIX;

    // transforms.xWidth = imageSize.width;
    // transforms.yWidth = imageSize.height;

    // DMATRIX M(votingPairs.size(), 4);
    // DMATRIX X(votingPairs.size(), 1);
    // DMATRIX Y(votingPairs.size(), 1);

    // for (int i = 0; i < votingPairs.size(); ++i)
    // {
    //     point_t& star = references[votingPairs[i].refStar];
    //     X(i, 0) = star.x / transforms.xWidth;
    //     Y(i, 0) = star.y / transforms.yWidth;
    // }

    // for (int i = 0; i < votingPairs.size(); ++i)
    // {
    //     point_t& star = targets[votingPairs[i].targetStar];
    //     double X = star.x / transforms.xWidth;
    //     double Y = star.y / transforms.yWidth;

    //     M(i, 0) = 1.0;
    //     M(i, 1) = X;
    //     M(i, 2) = Y;
    //     M(i, 3) = X * Y;
    // };

    // DMATRIX MT = ~M;
    // DMATRIX TM = MT * M;

    // try
    // {
    //     if (!TM.IsSingular())
    //     {
    //         DMATRIX A = !TM * MT * X;
    //         DMATRIX B = !TM * MT * Y;

    //         transforms.a0 = A(0, 0);
    //         transforms.a1 = A(1, 0);
    //         transforms.a2 = A(2, 0);
    //         transforms.a3 = A(3, 0);
    //         transforms.b0 = B(0, 0);
    //         transforms.b1 = B(1, 0);
    //         transforms.b2 = B(2, 0);
    //         transforms.b3 = B(3, 0);
    //     }
    // }
    // catch(math::matrix_error const&)
    // {
    //     return false;
    // }

    // return true;
}

//-----------------------------------------------------------------------------

double StarMatcher::validateTransformation(
    const voting_pair_list_t & testedPairs, Transformation& transforms
)
{
	double result = 0.0;

	// Compute the distance between the stars
	for (const auto& testedPair : testedPairs)
	{
		const point_t projected = transforms.transform(targets[testedPair.targetStar]);
		const double distance = projected.distance(references[testedPair.refStar]);

        if (distance > result)
            result = distance;
	}

	return result;
}
