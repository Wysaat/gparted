/* Copyright (C) 2004 Bart 'plors' Hakvoort
 * Copyright (C) 2010 Curtis Gedak
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "../include/OperationDelete.h"

namespace GParted
{

OperationDelete::OperationDelete( const Device & device, const Partition & partition_orig )
{
	type = OPERATION_DELETE ;

	this ->device = device ;
	this ->partition_original = partition_orig ;
}
	
void OperationDelete::apply_to_visual( std::vector<Partition> & partitions ) 
{
	if ( partition_original .inside_extended )
	{
		index_extended = find_index_extended( partitions ) ;
		
		if ( index_extended >= 0 )
			index = find_index_original( partitions[ index_extended ] .logicals ) ;

		if ( index >= 0 )
		{
			remove_original_and_adjacent_unallocated( partitions[ index_extended ] .logicals, index ) ;

			insert_unallocated( partitions[ index_extended ] .logicals,
					    partitions[ index_extended ] .sector_start,
					    partitions[ index_extended ] .sector_end,
					    device .sector_size,
					    true ) ;
		
			//if deleted partition was logical we have to decrease the partitionnumbers of the logicals
			//with higher numbers by one (only if its a real partition)
			if ( partition_original .status != GParted::STAT_NEW )
				for ( unsigned int t = 0 ; t < partitions[ index_extended ] .logicals .size() ; t++ )
					if ( partitions[ index_extended ] .logicals[ t ] .partition_number > 
					     partition_original .partition_number )
						partitions[ index_extended ] .logicals[ t ] .Update_Number(
							partitions[ index_extended ] .logicals[ t ] .partition_number -1 );
		}
	}
	else
	{
		index = find_index_original( partitions ) ;

		if ( index >= 0 )
		{
			remove_original_and_adjacent_unallocated( partitions, index ) ;
			
			insert_unallocated( partitions, 0, device .length -1, device .sector_size, false ) ;
		}
	}
}

void OperationDelete::create_description() 
{
	if ( partition_original.type == GParted::TYPE_LOGICAL )
		description = _("Logical Partition") ;
	else
		description = partition_original .get_path() ;

	/*TO TRANSLATORS: looks like   Delete /dev/hda2 (ntfs, 345 MiB) from /dev/hda */
	description = String::ucompose( _("Delete %1 (%2, %3) from %4"),
					description,
					Utils::get_filesystem_string( partition_original .filesystem ), 
					Utils::format_size( partition_original .get_sector_length(), partition_original .sector_size ),
					partition_original .device_path ) ;
}
	
void OperationDelete::remove_original_and_adjacent_unallocated( std::vector<Partition> & partitions, int index_orig ) 
{
	//remove unallocated space following the original partition
	if ( index_orig +1 < static_cast<int>( partitions .size() ) &&
	     partitions[ index_orig +1 ] .type == GParted::TYPE_UNALLOCATED )
		partitions .erase( partitions .begin() + index_orig +1 );
	
	//remove unallocated space preceding the original partition and the original partition
	if ( index_orig -1 >= 0 && partitions[ index_orig -1 ] .type == GParted::TYPE_UNALLOCATED )
		partitions .erase( partitions .begin() + --index_orig ) ;
				
	//and finally remove the original partition
	partitions .erase( partitions .begin() + index_orig ) ;
}

} //GParted

