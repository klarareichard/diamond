/****
DIAMOND protein aligner
Copyright (C) 2013-2017 Benjamin Buchfink <buchfink@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
****/

#include "align_range.h"
#include "../data/reference.h"
#include "../basic/config.h"
#include "seed_complexity.h"
#include "search.h"

double SeedComplexity::prob_[AMINO_ACID_COUNT];
const double SINGLE_INDEXED_SEED_SPACE_MAX_COVERAGE = 0.15;

void setup_search_cont()
{
	unsigned index_mode;
	if (config.mode_more_sensitive) {
		index_mode = 11;
	}
	else if (config.mode_sensitive) {
		index_mode = 11;
	}
	else {
		index_mode = 10;
		if(config.db_type != "nucl")
			Reduction::reduction = Reduction("KR EQ D N C G H F Y IV LM W P S T A");
        else{
            Reduction::reduction = Reduction("A C G T");
        }
	}
	::shapes = shape_config(index_mode, 1, vector<string>());
}

bool use_single_indexed(double coverage, size_t query_letters, size_t ref_letters)
{
	if (coverage >= SINGLE_INDEXED_SEED_SPACE_MAX_COVERAGE)
		return false;
	if (config.mode_more_sensitive || config.mode_sensitive) {
		return query_letters < 300000llu && query_letters * 20000llu < ref_letters;
	}
	else
		return query_letters < 3000000llu && query_letters * 2000llu < ref_letters;
}

void setup_search()
{
	if (config.algo == Config::double_indexed) {
		if (config.mode_very_sensitive) {
			Config::set_option(config.index_mode, 4u);
			Config::set_option(config.freq_sd, 200.0);
		} else if (config.mode_more_sensitive) {
			Config::set_option(config.index_mode, 9u);
			Config::set_option(config.freq_sd, 200.0);
		}
		else if (config.mode_sensitive) {
			Config::set_option(config.index_mode, 9u);
			Config::set_option(config.freq_sd, 10.0);
		}
		else {
			Config::set_option(config.index_mode, 8u);
			Config::set_option(config.freq_sd, 50.0);
		}
		if(config.command == Config::blastn){
			Reduction::reduction = Reduction(" A C G T");
		}else {
			Reduction::reduction = Reduction("A KR EDNQ C G H ILVM FYW P ST");
		}
		::shapes = shape_config(config.index_mode, config.shapes, config.shape_mask);
	}
	else {
		if (config.mode_more_sensitive) {
			Config::set_option(config.freq_sd, 200.0);
		}
		else if (config.mode_sensitive) {
			Config::set_option(config.freq_sd, 20.0);
		}
		else {
			Config::set_option(config.freq_sd, 50.0);
		}
		config.lowmem = 1;
	}

	SeedComplexity::init(Reduction::reduction);

	message_stream << "Algorithm: " << (config.algo == Config::double_indexed ? "Double-indexed" : "Query-indexed") << endl;
	verbose_stream << "Reduction: " << Reduction::reduction << endl;

	verbose_stream << "Seed frequency SD: " << config.freq_sd << endl;
	verbose_stream << "Shape configuration: " << ::shapes << endl;
	config.seed_anchor = std::min(::shapes[0].length_ - 1, 8u);
}

void setup_search_params(pair<size_t, size_t> query_len_bounds, size_t chunk_db_letters)
{
	const double b = config.min_bit_score == 0 ? score_matrix.bitscore(config.max_evalue, (unsigned)query_len_bounds.first) : config.min_bit_score;

	if (config.mode_very_sensitive) {
		if(config.command != Config::blastn){
			Reduction::reduction = Reduction("A KR EDNQ C G H ILVM FYW P ST"); // murphy.10
			Config::set_option(config.index_mode, 4u);
			::shapes = shape_config(config.index_mode, config.shapes, config.shape_mask);
			config.seed_anchor = std::min(::shapes[0].length_ - 1, 8u);
			Config::set_option(config.min_identities, 9u);
			Config::set_option(config.min_ungapped_score, 19.0);
			Config::set_option(config.window, 60u);
			Config::set_option(config.hit_band, 8);
			Config::set_option(config.min_hit_score, 23.0);
		}else{
			Reduction::reduction = Reduction("A C G T");
			Config::set_option(config.index_mode, 4u);
			::shapes = shape_config(config.index_mode, config.shapes, config.shape_mask);
			config.seed_anchor = std::min(::shapes[0].length_ - 1, 8u);
			Config::set_option(config.min_identities, 9u);
			Config::set_option(config.min_ungapped_score, 6.0);
			Config::set_option(config.window, 60u);
			Config::set_option(config.hit_band, 8);
			Config::set_option(config.min_hit_score, 8.0);
		}
		std::cout<<"min_identities = "<< config.min_identities << std::endl;
		std::cout<<"min_ungapped_score = "<< config.min_ungapped_score << std::endl;
		std::cout<<"min_hit_score = "<< config.min_hit_score << std::endl;
	}
	else {

		if(config.command != Config::blastn) {
			Config::set_option(config.min_identities, 11u);
			if (query_len_bounds.second <= 40) {
				//Config::set_option(config.min_identities, 10u);
				Config::set_option(config.min_ungapped_score, std::min(27.0, b));
			} else {
				//Config::set_option(config.min_identities, 9u);
				Config::set_option(config.min_ungapped_score, std::min(23.0, b));
			}

			if (query_len_bounds.second <= 80) {
				const int band = config.read_padding(query_len_bounds.second);
				Config::set_option(config.window, (unsigned) (query_len_bounds.second + band));
				Config::set_option(config.hit_band, band);
				Config::set_option(config.min_hit_score, b);
			} else {
				Config::set_option(config.window, 40u);
				Config::set_option(config.hit_band, 5);
				Config::set_option(config.min_hit_score, std::min(29.0, b));
			}
		}else{
			Config::set_option(config.min_identities, 9u);
			if (query_len_bounds.second <= 40) {
				//Config::set_option(config.min_identities, 10u);
				Config::set_option(config.min_ungapped_score, std::min(9.0, b));
			} else {
				//Config::set_option(config.min_identities, 9u);
				Config::set_option(config.min_ungapped_score, std::min(7.0, b));
			}

			if (query_len_bounds.second <= 80) {
				const int band = config.read_padding(query_len_bounds.second);
				Config::set_option(config.window, (unsigned) (query_len_bounds.second + band));
				Config::set_option(config.hit_band, band);
				Config::set_option(config.min_hit_score, b);
			} else {
				Config::set_option(config.window, 40u);
				Config::set_option(config.hit_band, 5);
				Config::set_option(config.min_hit_score, std::min(13.0, b));
			}

		}
		std::cout<<"min_identities = "<< config.min_identities << std::endl;
		std::cout<<"min_ungapped_score = "<< config.min_ungapped_score << std::endl;
		std::cout<<"min_hit_score = "<< config.min_hit_score << std::endl;
	}

	config.min_ungapped_raw_score = score_matrix.rawscore(config.min_ungapped_score);
	config.min_hit_raw_score = score_matrix.rawscore(config.min_hit_score);
	log_stream << "Query len bounds " << query_len_bounds.first << ' ' << query_len_bounds.second << endl;
	log_stream << "Search parameters " << config.min_ungapped_raw_score << ' ' << config.min_hit_score << ' ' << config.hit_cap << endl;
}
