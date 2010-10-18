/**
 *
 * Phantom OS - Phantom language library
 *
 * Copyright (C) 2005-2010 Dmitry Zavalishin, dz@dz.ru
 *
 * Internal: yes
 * Preliminary: yes
 *
 *
**/

package .internal;
import .ru.dz.phantom.handler;

/**
 *
 * This class has internal implementation (as everything in
 * .internal package). It means that VM will bever load its
 * bytecode, and internal version will be used instead. This
 * class definition must be synchronized with VM implementation.
 *
**/

/**
 *
 * OS level window object.
 * TODO: Must be dynamically resizeable.
 *
**/

class window
{
	int	getXSize() [16] {}
	int	getYSize() [17] {}

	int	getX() [18] {}
	int	getY() [19] {}


	void 	clear() [20] {}
	void 	fill(var fg : int) [21] {}

	void	setBg( var bg : int ) [22] {}
	void	setFg( var fg : int ) [23] {}

	// todo will need font parameter
	void	drawString( var x : int, var y : int, var s : string ) [24] {}
	void	drawImage( var x : int, var y : int, var img : string ) [25] {}

	void	setWinSize( var xsize : int, var ysize : int ) [26] {}
	void	setWinPosition( var x : int, var y : int ) [27] {}


 
	void	drawLine( var x : int, var y : int, var xsize : int, var ysize : int ) [28] {}
	void	drawBox( var x : int, var y : int, var xsize : int, var ysize : int ) [29] {}

	void	fillBox( var x : int, var y : int, var xsize : int, var ysize : int ) [30] {}
	void	fillEllipse( var x : int, var y : int, var xsize : int, var ysize : int ) [31] {}

	void	setEventHandler( var handler : .ru.dz.phantom.handler ) [32] {}
	void	setTitle( var title : string ) [33] {}

};
