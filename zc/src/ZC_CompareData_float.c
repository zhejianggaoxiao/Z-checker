#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>
#include "ZC_util.h"
#include "ZC_DataProperty.h"
#include "ZC_CompareData.h"
#include "zc.h"

void ZC_compareData_float(ZC_CompareData* compareResult, float* data1, float* data2, 
size_t r5, size_t r4, size_t r3, size_t r2, size_t r1)
{
	size_t i = 0;
	double minDiff = data2[0]-data1[0];
	double maxDiff = minDiff;
	double minErr = fabs(minDiff);
	double maxErr = minErr;
	double sum1 = 0, sum2 = 0, sumDiff = 0, sumErr = 0, sumErrSqr = 0;
	
	double minDiff_rel = 1E100;
	double maxDiff_rel = -1E100;
	double minErr_rel = 1E100;
	double maxErr_rel = 0;
	double sumDiff_rel = 0, sumErr_rel = 0, sumErrSqr_rel = 0;
	
	double err;
	size_t numOfElem = compareResult->property->numOfElem;
	double sumOfDiffSquare = 0, sumOfDiffSquare_rel = 0;
	size_t numOfElem_ = 0;

	double *diff = (double*)malloc(numOfElem*sizeof(double));
	double *relDiff = (double*)malloc(numOfElem*sizeof(double));

	for (i = 0; i < numOfElem; i++)
	{
		sum1 += data1[i];
		sum2 += data2[i];
		
		diff[i] = data2[i]-data1[i];
		if(minDiff > diff[i]) minDiff = diff[i];
		if(maxDiff < diff[i]) maxDiff = diff[i];
		sumDiff += diff[i];
		sumOfDiffSquare += diff[i]*diff[i];
				
		err = fabs(diff[i]);
		if(minErr>err) minErr = err;
		if(maxErr<err) maxErr = err;
		sumErr += err;
		sumErrSqr += err*err; //used for mse, nrmse, psnr
	
		if(data1[i]!=0)
		{
			numOfElem_ ++;
			relDiff[i] = diff[i]/data1[i];
			if(minDiff_rel > relDiff[i]) minDiff_rel = relDiff[i];
			if(maxDiff_rel < relDiff[i]) maxDiff_rel = relDiff[i];
			sumDiff_rel += relDiff[i];
			sumOfDiffSquare_rel += relDiff[i]*relDiff[i];
			
			err = fabs(relDiff[i]);
			if(minErr_rel>err) minErr_rel = err;
			if(maxErr_rel<err) maxErr_rel = err;
			sumErr_rel += err;
			sumErrSqr_rel += err*err;
		}	
	}
	
	ZC_DataProperty* property = compareResult->property;
	
	double zeromean_variance = property->zeromean_variance;
	double valRange = property->valueRange;
	double mean1 = sum1/numOfElem;
	double mean2 = sum2/numOfElem;
	
	double avgDiff = sumDiff/numOfElem;
	double avgErr = sumErr/numOfElem;
	double diffRange = maxDiff - minDiff;
	double mse = sumErrSqr/numOfElem;
	
	double avgErr_rel = sumErr_rel/numOfElem;
	double diffRange_rel = maxDiff_rel - minDiff_rel;
	double mse_rel = sumErrSqr_rel/numOfElem_;
	if(diffRange_rel>2*PWR_DIS_RNG_BOUND)
	{
		double avg = 0;//sumDiff_rel/numOfElem_;
		diffRange_rel = 2*PWR_DIS_RNG_BOUND;
		minDiff_rel = avg-PWR_DIS_RNG_BOUND;
		maxDiff_rel = avg+PWR_DIS_RNG_BOUND;
	}
	
	int index;
	
	if (minAbsErrFlag)
		compareResult->minAbsErr = minErr;

	if (minRelErrFlag)
		compareResult->minRelErr = minErr/valRange;

	if (maxAbsErrFlag)
		compareResult->maxAbsErr = maxErr;

	if (maxRelErrFlag)
		compareResult->maxRelErr = maxErr/valRange;

	if (avgAbsErrFlag)
		compareResult->avgAbsErr = avgErr;

	if (avgRelErrFlag)
		compareResult->avgRelErr = avgErr/valRange;
		
	compareResult->minPWRErr = minErr_rel;
	compareResult->maxPWRErr = maxErr_rel;
	compareResult->avgPWRErr = sumErr_rel/numOfElem_;

	if (absErrPDFFlag)
	{
		double interval = diffRange/PDF_INTERVALS;
		double *absErrPDF = NULL;
		if(interval==0)
		{
			absErrPDF = (double*)malloc(sizeof(double));
			*absErrPDF = 0;
		}
		else
		{
			absErrPDF = (double*)malloc(sizeof(double)*PDF_INTERVALS);
			memset(absErrPDF, 0, PDF_INTERVALS*sizeof(double));

			for (i = 0; i < numOfElem; i++)
			{
				index = (int)((diff[i]-minDiff)/interval);
				if(index==PDF_INTERVALS)
					index = PDF_INTERVALS-1;
				absErrPDF[index] += 1;
			}

			for (i = 0; i < PDF_INTERVALS; i++)
				absErrPDF[i]/=numOfElem;			
		}
		compareResult->absErrPDF = absErrPDF;
		compareResult->err_interval = interval;
		compareResult->err_minValue = minDiff;		
	}

	if (pwrErrPDFFlag)
	{
		double interval = diffRange_rel/PDF_INTERVALS_REL;
		double *relErrPDF = NULL;
		if(interval==0)
		{
			relErrPDF = (double*)malloc(sizeof(double));
			*relErrPDF = 0;
		}
		else
		{
			relErrPDF = (double*)malloc(sizeof(double)*PDF_INTERVALS_REL);
			memset(relErrPDF, 0, PDF_INTERVALS_REL*sizeof(double));

			for (i = 0; i < numOfElem; i++)
			{
				if(data1[i]!=0)
				{
					if(relDiff[i]>maxDiff_rel)
						relDiff[i] = maxDiff_rel;
					if(relDiff[i]<minDiff_rel)
						relDiff[i] = minDiff_rel;
					index = (int)((relDiff[i]-minDiff_rel)/interval);
					if(index==PDF_INTERVALS_REL)
						index = PDF_INTERVALS_REL-1;
					relErrPDF[index] += 1;					
				}
			}
			for (i = 0; i < PDF_INTERVALS_REL; i++)
				relErrPDF[i]/=numOfElem_;			
		}
		compareResult->pwrErrPDF = relErrPDF;
		compareResult->err_interval_rel = interval;
		compareResult->err_minValue_rel = minDiff_rel;		
	}

	if (autoCorrAbsErrFlag)
	{
		double *autoCorrAbsErr = (double*)malloc((AUTOCORR_SIZE+1)*sizeof(double));

		size_t delta;

		if (numOfElem > 4096)
		{
			double covDiff = 0;
			for (i = 0; i < numOfElem; i++)
			{
				covDiff += (diff[i] - avgDiff)*(diff[i] - avgDiff);
			}

			covDiff = covDiff/numOfElem;

			if (covDiff == 0)
			{
				for (delta = 1; delta <= AUTOCORR_SIZE; delta++)
					autoCorrAbsErr[delta] = 0;
			}
			else
			{
				for (delta = 1; delta <= AUTOCORR_SIZE; delta++)
				{
					double sum = 0;

					for (i = 0; i < numOfElem-delta; i++)
					{
						sum += (diff[i]-avgDiff)*(diff[i+delta]-avgDiff);
					}

					autoCorrAbsErr[delta] = sum/(numOfElem-delta)/covDiff;
				}
			}
		}
		else
		{
			for (delta = 1; delta <= AUTOCORR_SIZE; delta++)
			{
				double avg_0 = 0;
				double avg_1 = 0;

				for (i = 0; i < numOfElem-delta; i++)
				{
					avg_0 += diff[i];
					avg_1 += diff[i+delta];
				}

				avg_0 = avg_0 / (numOfElem-delta);
				avg_1 = avg_1 / (numOfElem-delta);

				double cov_0 = 0;
				double cov_1 = 0;

				for (i = 0; i < numOfElem-delta; i++)
				{
					cov_0 += (diff[i] - avg_0) * (diff[i] - avg_0);
					cov_1 += (diff[i+delta] - avg_1) * (diff[i+delta] - avg_1);
				}

				cov_0 = cov_0/(numOfElem-delta);
				cov_1 = cov_1/(numOfElem-delta);

				cov_0 = sqrt(cov_0);
				cov_1 = sqrt(cov_1);


				if (cov_0*cov_1 == 0)
				{
					for (delta = 1; delta <= AUTOCORR_SIZE; delta++)
						autoCorrAbsErr[delta] = 0;
				}
				else
				{
					double sum = 0;

					for (i = 0; i < numOfElem-delta; i++)
						sum += (diff[i]-avg_0)*(diff[i+delta]-avg_1);

					autoCorrAbsErr[delta] = sum/(numOfElem-delta)/(cov_0*cov_1);
				}
			}

		}
        
        autoCorrAbsErr[0] = 1;
		compareResult->autoCorrAbsErr = autoCorrAbsErr;
	}

	if (pearsonCorrFlag)
	{
		double prodSum = 0, sum1 = 0, sum2 = 0;
    	for (i = 0; i < numOfElem; i++)
    	{
    		prodSum += (data1[i]-mean1)*(data2[i]-mean2);
    		sum1 += (data1[i]-mean1)*(data1[i]-mean1);
    		sum2 += (data2[i]-mean2)*(data2[i]-mean2);
    	}

    	double std1 = sqrt(sum1/numOfElem);
    	double std2 = sqrt(sum2/numOfElem);
    	double ee = prodSum/numOfElem;
    	double pearsonCorr = 0;

    	if (std1*std2 != 0)
    		pearsonCorr = ee/std1/std2;

    	compareResult->pearsonCorr = pearsonCorr;
	}

	if (rmseFlag)
	{
		double rmse = sqrt(mse);
		compareResult->rmse = rmse;
	}

	if (nrmseFlag)
	{
		double nrmse = sqrt(mse)/valRange;
		compareResult->nrmse = nrmse;
	}

	if(snrFlag)
	{
		compareResult->snr = 10*log10(zeromean_variance/mse);
		//printf("compareResult->snr=%f ccompareResult->snr_db=%f", compareResult->snr, compareResult->snr_db); 		
	}

	if (psnrFlag)
	{
		double psnr = -20.0*log10(sqrt(mse)/valRange);
		compareResult->psnr = psnr;
	}

	if (valErrCorrFlag)
	{
		double prodSum = 0, sum1 = 0, sumDiff = 0;
    	for (i = 0; i < numOfElem; i++)
    	{
    		prodSum += (data1[i]-mean1)*(diff[i]-avgDiff);
    		sum1 += (data1[i]-mean1)*(data1[i]-mean1);
    		sumDiff += (diff[i]-avgDiff)*(diff[i]-avgDiff);
    	}

    	double std1 = sqrt(sum1/numOfElem);
    	double stdDiff = sqrt(sumDiff/numOfElem);
    	double ee = prodSum/numOfElem;
      	double valErrCorr = 0;

    	if (std1*stdDiff != 0)
    		valErrCorr = ee/std1/stdDiff;

		compareResult->valErrCorr = valErrCorr;
	}

	free(diff);
	free(relDiff);

}
