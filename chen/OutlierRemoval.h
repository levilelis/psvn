/*
 * OutlierRemoval.h
 *
 *  Created on: Dec 19, 2013
 *      Author: levilelis
 */

#ifndef OUTLIERREMOVAL_H_
#define OUTLIERREMOVAL_H_

#include <algorithm>

struct value_var_reduction {
	double value;
	double var_reduction;
};

bool operator< (const value_var_reduction& o1, const value_var_reduction& o2)
{
	if(o1.var_reduction != o2.var_reduction)
	{
		return o1.var_reduction > o2.var_reduction;
	}

	if(o1.value != o2.value)
	{
		return o1.value > o2.value;
	}

	return false;
}

class OutlierRemoval {

private:
	double compute_mean( vector<double>& v )
	{
		double mean = 0;

		for( int i = 0; i < v.size(); i++ )
		{
			mean += v[i];
		}

		return mean / v.size();
	}

	double compute_variance( vector<double>& v )
	{
		double mean = compute_mean( v );
		double var = 0;

		for( int i = 0; i < v.size(); i++ )
		{
			var += pow(v[i] - mean, 2);
		}

		var = var / v.size();

		return var;
	}

	void getVectorSortedByVarianceChange( vector<double>& v )
	{
		vector<value_var_reduction> v2;
		double initial_var = compute_variance( v );

		for( int i = 0; i < v.size(); i++ )
		{
			vector<double> v_copy;

			for( int j = 0; j < v.size(); j++ )
			{
				if( j != i )
				{
					v_copy.push_back( v[j] );
				}
			}

			double var = compute_variance( v_copy );
			value_var_reduction tuple;
			tuple.value = v[i];
			tuple.var_reduction = initial_var - var;

			v2.push_back( tuple );
		}

		sort(v2.begin(), v2.end());

		v.clear();

		for( int i = 0; i < v2.size(); i++ )
		{
		//	cout << "value: " << v2[i].value << " var red: " << v2[i].var_reduction << endl;
			v.push_back( v2[i].value );
		}
		//cout << endl;
	}

public:
	void remove_outliers( vector<double>& v )
	{
		sort(v.begin(), v.end());

		double initial_var = compute_variance( v );

		double previous_sf = 0;
		double mean = 0;

		getVectorSortedByVarianceChange( v );

		//cout << "==========" << endl;
		while(v.size() > 0)
		{
			double being_removed = v[0];
			v.erase(v.begin());
			mean = compute_mean(v);
			double current_var = compute_variance(v);

			double sf = v.size() * (initial_var - current_var);
			if( sf < previous_sf )
			{
				v.push_back( being_removed );
				break;
			}

			//cout << "Removing: " << being_removed << endl;

			previous_sf = sf;
		}
		//cout << "==========" << endl;

	}

};


#endif /* OUTLIERREMOVAL_H_ */
