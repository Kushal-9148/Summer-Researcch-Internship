/*
**********************************************************
The code simulates the working of DIP cache replacement policy.


Assumptions:
16 Way set associative cache.
Cache Size: 1MB
Line Size: 64B
Set size : 16 x 64B => 1KB
Number of Sets in cache => (1MB / 1KB) => 1024 Sets. (These parameters can be tweaked).
We are dealing with a Byte Addressable system.
Epsilon Parameter for BIP is 32.

Set Duelling:
Every 32nd Set is allocated to LRU.
Every 33rd Set is allocated to BIP.
**********************************************************
*/



#include <iostream>
#include <unordered_map>
#include <vector>
#include <cmath>
#include <list>
using namespace std;


class Cache_Simulator
{
	public:
	int PSEL;	            //PSEL is policy selector.
	int Epsilon;	        //Epsilon Parameter for BIP.
	int Hits;
	long long no_of_set;
	long long cache_size;
	long long no_of_simulation;
	long long working_set;

	Cache_Simulator(long long cache_size, long long set_associativity, long long line_size)
	{

		Hits = 0;
		Epsilon = 0;
		PSEL = 512;
		cache_size = cache_size;
		no_of_set = cache_size / (set_associativity * line_size);
	}

	bool LRU(long long set, long long tag, vector<list < long long>> &cache)
	{
		//The function updates the cache using LRU policy.

		bool hit = 0;

		//Traverse the entire list to find if the tag matches.

		list < long long>::iterator it, LRU = (cache[set].end()), MRU = cache[set].begin();

		for (it = cache[set].begin(); it != cache[set].end(); ++it)
		{

			if (*it == tag)
			{
				//If demanded tag equals to the value in the cache its a hit.
				hit = 1;

				//Delete the tag from that position.
				cache[set].erase(it);

				//Place the tag in MRU position.
				cache[set].insert(cache[set].begin(), tag);

				//Acknowledge the hit and return.
				return hit;
			}
		}

		//In case the function do not possess the demanded tag
		//Check if the list is full

		if (cache[set].size() == 16)
		{
			//Set associativity is 16.
			//Remove tag at LRU position.
			--LRU;
			cache[set].erase(LRU);

			//Insert the new at MRU position
			cache[set].insert(MRU, tag);
		}
		else
		{
			//Insert at MRU position.
			cache[set].insert(MRU, tag);
		}

		return hit;
	}

	bool BIP(long long set, long long tag, vector<list < long long >> &cache)
	{
		bool hit = 0;
		list < long long>::iterator it, LRU = (cache[set].end()), MRU = cache[set].begin();

		--LRU;

		for (it = cache[set].begin(); it != cache[set].end(); ++it)
		{
			if (*it == tag)
			{
				//If demanded tag equals to the value in the cache its a hit.
				hit = 1;

				//Delete the tag from that position.
				cache[set].erase(it);

				//Place the tag in MRU position.
				cache[set].insert(cache[set].begin(), tag);

				//Acknowledge the hit and return.
				return hit;
			}
		}
		//If it is not a hit, increament the Epsilon.
		++Epsilon;
		if (Epsilon == 32)
		{
			Epsilon = 0;
		}

		//Check the size of the list.
		if (cache[set].size() == 16)
		{
			//Evict the tag at LRU position.
			cache[set].erase(LRU);
			LRU = cache[set].end();
		}

		//If Epsilon is 0 insert in MRU position
		if (Epsilon == 0)
		{
			cache[set].insert(MRU, tag);
		}
		else
		{
			//Insert at LRU position
			cache[set].insert(LRU, tag);
		}
		return hit;
	}

	bool DIP(long long set, long long tag, vector<list < long long>> &cache)
	{

		if (PSEL <= 512)
		{
			return LRU(set, tag, cache);
		}
		else
		{
			return BIP(set, tag, cache);
		}
	}

	void cache_access(long long data_adr, vector<list < long long>> &cache)
	{
		//Since we have 64B line size we can remove 6 offset bits from the data_addr.
		data_adr = data_adr >> 6;


		//Last 10 bits are index bits and they would determine the set.
		long long set_no = data_adr % (no_of_set);


		// The remaining are the tag bits used to identify a line in given set.
		long long tag = data_adr / (no_of_set);


		if (set_no % 32 == 0)
		{
			// Check if set is dedicated to LRU policy
			if ( !LRU(set_no, tag, cache) )
			{
				//PSEL is a saturating counter so it shall not go beyond 1023
				++PSEL;
			}
			else
			{ 	
			    ++Hits;
			}
		}
		else if (set_no % 32 == 1)
		{
			// Check if set is dedicated to BIP policy
			if (!BIP(set_no, tag, cache))
			{
				if (PSEL > 0)
				{
					--PSEL;
                    
				}
			}
			else
			{ 
			    ++Hits;
			}
		}
		else
		{
			//Otherwise the set is follower set.
			if (DIP(set_no, tag, cache))
			{ 
           			     ++Hits;
			}
		}
	}

	long long demand_line_simulator(long long cache_size, long long set_associativity, long long line_size)
	{

		//Calculate number of sets
		vector<list < long long>> cache(no_of_set);

		printf("Enter the number of stimulations\n");
		cin >> no_of_simulation;
		
		
		printf("Enter the size of working set\n");
		cin >> working_set;
		
		
		for (int j = 0; j < no_of_simulation; j++)
		{
			for (int i = 0; i < working_set; ++i)
			{
				long long data_adr = (i * 64);				        
				cache_access(data_adr, cache);
			}
		}

		return Hits;
	}
};

int main()
{
	long long cache_size = 1 << 20, set_associativity = 16, line_size = 64;	//Default cache_size , set_associativity , line_size; the size is byte addressable.
	printf("Please Enter Cahce Size, Set Associativity and Line Size\n");
	cin >> cache_size >> set_associativity >> line_size;

	class Cache_Simulator cache(cache_size, set_associativity, line_size);

	cache.demand_line_simulator(cache_size, set_associativity, line_size);

	cout << "Total hits are :" << cache.Hits << "\n";
    
	cout << "Hit percentage is :" << (cache.Hits * 100) / (cache.no_of_simulation * cache.working_set) << "\n";

	return 0;
}
