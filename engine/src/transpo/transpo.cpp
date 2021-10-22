#include "transpo.h"

transpo::transpo()
{
}

transpo::~transpo()
{
}

bool transpo::allocate(const size_t& size_in_mb)
{
	if (this->allocated) { return false; }
	this->tt = (ptransposition_table)malloc(size_in_mb * 0x100000);
	this->size_in_mb = size_in_mb;
	this->entries = ((size_in_mb * 0x100000) / sizeof(transposition_table));
	this->allocated = true;
	return true;
}

bool transpo::deallocate()
{
	if (!this->allocated) { return false; }
	free(this->tt);
	this->size_in_mb = 0;
	this->entries = 0;
	this->allocated = false;
	return true;
}

bool transpo::reset()
{
	if (!this->allocated || !this->size_in_mb) { return false; }

	ptransposition_table entry;

	for (entry = this->tt; entry < (this->tt + this->entries); entry++)
	{
		entry->hashkey = 0ULL;
		entry->depth = 0;
		entry->flag = 0;
		entry->score = 0;
	}
	return true;
}

int transpo::read(const uint64_t& hashkey, const int& depth, const int& alpha, const int& beta)
{
	ptransposition_table entry = &this->tt[hashkey % this->entries];

	if (!entry) { return VALUE_UNKNOWN; }

	if (entry->hashkey == hashkey)
	{
		if (entry->depth == depth)
		{
			int score = entry->score;

			if (entry->flag == tf_exact) { return score; }

			if ((entry->flag == tf_alpha) && (score <= alpha)) { return alpha; }

			if ((entry->flag == tf_beta) && (score >= beta)) { return beta; }
		}
	}

	return VALUE_UNKNOWN;
}

bool transpo::write(uint64_t hashkey, int depth, uint8_t flag, int score)
{
	ptransposition_table entry = &this->tt[hashkey % this->entries];

	if (!entry) { return false; }

	entry->hashkey = hashkey;
	entry->depth = depth;
	entry->flag = flag;
	entry->score = score;

	return true;
}
