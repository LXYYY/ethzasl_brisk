//
//    nonMaximumSuppression - this nonMaximumSuppression is a re-implementation
//                            of the NMS as proposed by Rosten. However, this
//                            implementation is complete and about 40% faster
//                            than the OpenCV-version
//
//    Copyright (C) 2010  Elmar Mair
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <stdlib.h>
#include <agast/wrap-opencv.h>
#include <agast/ast-detector.h>

namespace agast {

void AstDetector::nonMaximumSuppression(const std::vector<cv::KeyPoint>& corners_all,
                                        std::vector<cv::KeyPoint>& corners_nms) {
  int currCorner_ind;
  int lastRow = 0, next_lastRow = 0;
  std::vector<cv::KeyPoint>::const_iterator currCorner;
  int lastRowCorner_ind = 0, next_lastRowCorner_ind = 0;
  std::vector<int>::iterator nmsFlags_p;
  std::vector<cv::KeyPoint>::iterator currCorner_nms;
  int j;
  int numCorners_all = corners_all.size();
  int nMaxCorners = corners_nms.capacity();

  currCorner = corners_all.begin();

  if (numCorners_all > nMaxCorners) {
    if (nMaxCorners == 0) {
      nMaxCorners = 512 > numCorners_all ? 512 : numCorners_all;
      corners_nms.reserve(nMaxCorners);
      nmsFlags.reserve(nMaxCorners);
    } else {
      nMaxCorners *= 2;
      if (numCorners_all > nMaxCorners)
        nMaxCorners = numCorners_all;
      corners_nms.reserve(nMaxCorners);
      nmsFlags.reserve(nMaxCorners);
    }
  }
  corners_nms.resize(numCorners_all);
  nmsFlags.resize(numCorners_all);

  nmsFlags_p = nmsFlags.begin();
  currCorner_nms = corners_nms.begin();

  // Set all flags to MAXIMUM.
  for (j = numCorners_all; j > 0; j--)
    *nmsFlags_p++ = -1;
  nmsFlags_p = nmsFlags.begin();

  for (currCorner_ind = 0; currCorner_ind < numCorners_all; currCorner_ind++) {
    int t;

    // Check above.
    if (lastRow + 1 < currCorner->pt.y) {
      lastRow = next_lastRow;
      lastRowCorner_ind = next_lastRowCorner_ind;
    }
    if (next_lastRow != currCorner->pt.y) {
      next_lastRow = currCorner->pt.y;
      next_lastRowCorner_ind = currCorner_ind;
    }
    if (lastRow + 1 == currCorner->pt.y) {
      // Find the corner above the current one.
      while ((corners_all[lastRowCorner_ind].pt.x < currCorner->pt.x)
          && (corners_all[lastRowCorner_ind].pt.y == lastRow))
        lastRowCorner_ind++;

      if ((corners_all[lastRowCorner_ind].pt.x == currCorner->pt.x)
          && (lastRowCorner_ind != currCorner_ind)) {
        int t = lastRowCorner_ind;
        while (nmsFlags[t] != -1)  // Find the maximum in this block.
          t = nmsFlags[t];

        if (scores[currCorner_ind] < scores[t]) {
          nmsFlags[currCorner_ind] = t;
        } else
          nmsFlags[t] = currCorner_ind;
      }
    }

    // Check left.
    t = currCorner_ind - 1;
    if ((currCorner_ind != 0) && (corners_all[t].pt.y == currCorner->pt.y)
        && (corners_all[t].pt.x + 1 == currCorner->pt.x)) {
      int currCornerMaxAbove_ind = nmsFlags[currCorner_ind];

      while (nmsFlags[t] != -1)  // Find the maximum in that area.
        t = nmsFlags[t];

      if (currCornerMaxAbove_ind == -1)  // No maximum above.
          {
        if (t != currCorner_ind) {
          if (scores[currCorner_ind] < scores[t])
            nmsFlags[currCorner_ind] = t;
          else
            nmsFlags[t] = currCorner_ind;
        }
      } else	// Maximum above.
      {
        if (t != currCornerMaxAbove_ind) {
          if (scores[currCornerMaxAbove_ind] < scores[t]) {
            nmsFlags[currCornerMaxAbove_ind] = t;
            nmsFlags[currCorner_ind] = t;
          } else {
            nmsFlags[t] = currCornerMaxAbove_ind;
            nmsFlags[currCorner_ind] = currCornerMaxAbove_ind;
          }
        }
      }
    }

    currCorner++;
  }

  // Collecting maximum corners.
  corners_nms.resize(0);
  for (currCorner_ind = 0; currCorner_ind < numCorners_all; currCorner_ind++) {
    if (*nmsFlags_p++ == -1)
      corners_nms.push_back(corners_all[currCorner_ind]);
  }
}
}  // namespace agast
