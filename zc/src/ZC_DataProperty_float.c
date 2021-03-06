#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <sys/stat.h>
#include "ZC_DataProperty.h"
#include "zc.h"
#include "iniparser.h"

#ifdef HAVE_MPI
ZC_DataProperty* ZC_genProperties_float_online(char* varName, float *data, size_t numOfElem, size_t r5, size_t r4, size_t r3, size_t r2, size_t r1)
{
	size_t i = 0;
	ZC_DataProperty* property = (ZC_DataProperty*)malloc(sizeof(ZC_DataProperty));
	memset(property, 0, sizeof(ZC_DataProperty));
	
	property->varName = varName;
	property->dataType = ZC_FLOAT;
	property->data = data;
	
	property->numOfElem = numOfElem;
	double min=data[0],max=data[0],sum=0;

    for(i=0;i<numOfElem;i++)
	{
		if(min>data[i]) min = data[i];
		if(max<data[i]) max = data[i];
		sum += data[i];
	}
	
	if(globalDataLength>0)
		property->numOfElem = globalDataLength;
	else
	{
		MPI_Allreduce(&numOfElem, &globalDataLength, 1, MPI_LONG, MPI_SUM, ZC_COMM_WORLD);
		property->numOfElem = globalDataLength;
	}
	MPI_Allreduce(&min, &property->minValue, 1, MPI_DOUBLE, MPI_MIN, ZC_COMM_WORLD);
	MPI_Allreduce(&max, &property->maxValue, 1, MPI_DOUBLE, MPI_MAX, ZC_COMM_WORLD);
	MPI_Allreduce(&sum, &property->avgValue, 1, MPI_DOUBLE, MPI_SUM, ZC_COMM_WORLD);
	property->avgValue = property->avgValue/globalDataLength;
	
	//compute zeromean_variance for the following computation of SNR
	double med = property->minValue + (property->maxValue - property->minValue)/2;
	double sum_of_square = 0;
	for(i=0;i<numOfElem;i++)
		sum_of_square += (data[i] - med)*(data[i] - med);
	MPI_Allreduce(&sum_of_square, &property->zeromean_variance, 1, MPI_DOUBLE, MPI_SUM, ZC_COMM_WORLD);
	property->zeromean_variance = property->zeromean_variance/globalDataLength;
	property->valueRange = property->maxValue - property->minValue;
	
	if(entropyFlag)
	{
		double absErr = 1E-3 * property->valueRange; /*TODO change fixed value to user input*/
		double entVal = 0.0;
		size_t table_size;
		table_size = (size_t)(property->valueRange/absErr)+1;	
		long *table = (long*)malloc(table_size*sizeof(long));
		memset(table, 0, table_size*sizeof(long));
		long *gtable = (long*)malloc(table_size*sizeof(long));
		memset(gtable, 0, table_size*sizeof(long));
		
		unsigned long index = 0;
		for (i = 0; i < numOfElem; i++)
 		{
			index = (unsigned long)((data[i]-min)/absErr);
			table[index]++;
		}
 
		MPI_Reduce(table, gtable, table_size, MPI_LONG, MPI_SUM, 0, ZC_COMM_WORLD);
 
		if(myRank==0)
		{
			for (i = 0; i<table_size; i++)
				if (gtable[i] != 0)
				{
					double prob = (double)gtable[i]/globalDataLength;
					entVal -= prob*log(prob)/log(2);
				}
		}

		property->entropy = entVal;
		free(table);
		free(gtable);
	}
	if(autocorrFlag)
	{
		double ccov[AUTOCORR_SIZE+1], gcov[AUTOCORR_SIZE+1];
		memset(ccov, 0, sizeof(double)*(AUTOCORR_SIZE+1));
		memset(gcov, 0, sizeof(double)*(AUTOCORR_SIZE+1));
		
		double *autocorr = (double*)malloc((AUTOCORR_SIZE+1)*sizeof(double));
		int delta;
		double var = 0, gvar = 0;
		for (i = 0; i < numOfElem; i++)
			var += (data[i] - property->avgValue)*(data[i] - property->avgValue);
		
		for(delta = 1; delta <= AUTOCORR_SIZE; delta++)
		{
			double cov = 0;
			for (i = 0; i < numOfElem-delta; i++)
				cov += (data[i] - property->avgValue)*(data[i+delta] - property->avgValue);
			ccov[delta] = cov;	
		}
		
		MPI_Reduce(&var, &gvar, 1, MPI_DOUBLE, MPI_SUM, 0, ZC_COMM_WORLD);
		MPI_Reduce(ccov, gcov, AUTOCORR_SIZE+1, MPI_DOUBLE, MPI_SUM, 0, ZC_COMM_WORLD);
		
		if(myRank==0)
		{	
			gvar = gvar/globalDataLength;
			if (gvar == 0)
			{
				for (delta = 1; delta <= AUTOCORR_SIZE; delta++)
					autocorr[delta] = 0;
			}
			else
			{
				for (delta = 1; delta <= AUTOCORR_SIZE; delta++)
				{
					autocorr[delta] = gcov[delta]/(globalDataLength-nbProc*delta)/gvar;				
				}
			}
			autocorr[0] = 1;
			property->autocorr = autocorr;					
		}
	}
	//TODO compute FFT 
	
	
	//TODO compute Lap
	
	return property;
}
#endif

ZC_DataProperty* ZC_genProperties_float(char* varName, float *data, size_t numOfElem, size_t r5, size_t r4, size_t r3, size_t r2, size_t r1)
{
	size_t i = 0;
	ZC_DataProperty* property = (ZC_DataProperty*)malloc(sizeof(ZC_DataProperty));
	memset(property, 0, sizeof(ZC_DataProperty));
	
	property->varName = varName;
	property->dataType = ZC_FLOAT;
	property->data = data;

	property->numOfElem = numOfElem;
	double min=data[0],max=data[0],sum=0,avg,valueRange;

    for(i=0;i<numOfElem;i++)
	{
		if(min>data[i]) min = data[i];
		if(max<data[i]) max = data[i];
		sum += data[i];
	}

	double med = min+(max-min)/2;
	double sum_of_square = 0;
	for(i=0;i<numOfElem;i++)
		sum_of_square += (data[i] - med)*(data[i] - med);
	property->zeromean_variance = sum_of_square/numOfElem;

	avg = sum/numOfElem;
	valueRange = max - min;

	if (minValueFlag)
		property->minValue = min;

	if (maxValueFlag)
		property->maxValue = max;

	if (avgValueFlag)
		property->avgValue = avg;
	
	if (valueRangeFlag)
		property->valueRange = valueRange;
    
	if(entropyFlag)
	{
		double absErr = 1E-3 * valueRange; /*TODO change fixed value to user input*/
		double entVal = 0.0;
		size_t table_size;
		table_size = (size_t)(valueRange/absErr)+1;	
		size_t *table = (size_t*)malloc(table_size*sizeof(size_t));
		memset(table, 0, table_size*sizeof(size_t));
		
		unsigned long index = 0;
		for (i = 0; i < numOfElem; i++)
 		{
			index = (unsigned long)((data[i]-min)/absErr);
			table[index]++;
		}
		for (i = 0; i<table_size; i++)
			if (table[i] != 0)
			{
				double prob = (double)table[i]/numOfElem;
				entVal -= prob*log(prob)/log(2);
			}

		property->entropy = entVal;
		free(table);
	}

	if(autocorrFlag)
	{
		double *autocorr = (double*)malloc((AUTOCORR_SIZE+1)*sizeof(double));

		int delta;

		if (numOfElem > 4096)
		{
			double cov = 0;
			for (i = 0; i < numOfElem; i++)
				cov += (data[i] - avg)*(data[i] - avg);

			cov = cov/numOfElem;

			if (cov == 0)
			{
				for (delta = 1; delta <= AUTOCORR_SIZE; delta++)
					autocorr[delta] = 0;
			}
			else
			{
				for(delta = 1; delta <= AUTOCORR_SIZE; delta++)
				{
					double sum = 0;

					for (i = 0; i < numOfElem-delta; i++)
						sum += (data[i]-avg)*(data[i+delta]-avg);
						
					autocorr[delta] = sum/(numOfElem-delta)/cov;
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
					avg_0 += data[i];
					avg_1 += data[i+delta];
				}

				avg_0 = avg_0 / (numOfElem-delta);
				avg_1 = avg_1 / (numOfElem-delta);

				double cov_0 = 0;
				double cov_1 = 0;

				for (i = 0; i < numOfElem-delta; i++)
				{
					cov_0 += (data[i]-avg_0)*(data[i]-avg_0);
					cov_1 += (data[i+delta]-avg_1)*(data[i+delta]-avg_1);
				}

				cov_0 = cov_0/(numOfElem-delta);
				cov_1 = cov_1/(numOfElem-delta);

				cov_0 = sqrt(cov_0);
				cov_1 = sqrt(cov_1);

				if (cov_0*cov_1 == 0)
				{
					for (delta = 1; delta <= AUTOCORR_SIZE; delta++)
						autocorr[delta] = 0;
				}
				else
				{
					double sum = 0;

					for (i = 0; i < numOfElem-delta; i++)
						sum += (data[i]-avg_0)*(data[i+delta]-avg_1);

					autocorr[delta] = sum/(numOfElem-delta)/(cov_0*cov_1);
				}
			}
		}

		autocorr[0] = 1;
		property->autocorr = autocorr;
	}
	
	if(fftFlag)
	{
        int fft_size = pow(2, (int)log2(numOfElem));
        property->fftCoeff = ZC_computeFFT(data, fft_size, ZC_FLOAT);
	}
	
	if (lapFlag)
	{
		double *lap = (double*)malloc(numOfElem*sizeof(double));
		double *ddata = (double*)malloc(numOfElem*sizeof(double));
		for(i=0;i<numOfElem;i++)
			ddata[i] = data[i];
		computeLap(ddata, lap, r5, r4, r3, r2, r1);
		free(ddata);
		property->lap = lap;
	}

	return property;
}
