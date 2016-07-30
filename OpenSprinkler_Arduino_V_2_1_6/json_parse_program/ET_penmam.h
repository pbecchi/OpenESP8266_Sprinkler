#pragma once
#include <Arduino.h>
#include <math.h>
#define PI 3.1415
float atmos_pres(int alt) {
	/*
		Calculates atmospheric pressure(kPa) using equation(7) in
		the FAO paper, page 62. Calculated using a simplification
		of the ideal gas law, assuming 20 deg C for a standard atmosphere.

		Arguments :
		alt - elevation / altitude above sea level(m)
		

	if (alt < -20 || alt > 11000) {
		SPFL_D("alt=%d is not in range -20 to 11000 m", alt);
	}*/
		float tmp1 = (293.0 - (0.0065 * alt)) / 293.0;
		float tmp2 = pow(tmp1, 5.26);
		float atmos_pres = 101.3 * tmp2;
		return atmos_pres;
}

float sunset_hour_angle(float lat, float sd) {
	/*"""
	Calculates sunset hour angle[rad] from latitude and solar
	declination using FAO equation 25.

	Arguments :
	lat - latitude[decimal degrees] Note : value should be negative if it is
	degrees south, positive if degrees north
	sd - solar declination[rad]
	"""
	# TODO : Raise exception for sd
	# Raise exceptions
	if (lat < -90.0 or lat > 90.0) :
		raise ValueError, 'latitude=%g is not in range -90 - 906' %lat

		# Convert latitude from decimal degrees to radians*/
	float lat_rad = lat  * (PI / 180);

//		# Calculate sunset hour angle(sha)[radians] from latitude and solar
//		# declination using FAO equation 25

	float sha = acos(-tan(lat_rad) *tan(sd));
	return sha;
}

float daily_mean_t(float tmin, float tmax) { return (tmin + tmax) / 2.; }

float daily_soil_heat_flux(float t_cur, float t_prev, float delta_t, float soil_heat_cap = 2.1, float delta_z = 0.10) {
	/*
		Estimates the daily soil heat flux(Gday)[MJ m - 2 day - 1]
		assuming a grass crop from the curent air temperature
		and the previous air temperature.The length over time over which the
		current and previous air temperatures are measured are specified by t_len
		which should be greater than 1 day.The calculations are based on FAO
		equation 41. The soil heat capacity is related to its mineral composition
		and water content.The effective soil depth(z) is only 0.10 - 0.20 m for one
		day.The resluting heat flux can be converted to
		equivalent evaporation[mm day - 1] using the equiv_evap() function.

		Arguments:
	t_cur - air temperature at tim i(current)[deg C]
	t_prev - air temperature at time i - 1[deg C]
	delta_t - length of time interval between t_cur and t_prev[day]
	soil_heat_cap - soil heat capacity[MJ m - 3 degC - 1](default value is 2.1)
	delta_z - effective soil depth[m](default - 0.1 m following FAO
		recommendation for daily calculations
		
		//# Raise exceptions
	if (t_prev < -95.0 || t_prev > 60.0) {
		raise ValueError, 't_prev=%g is not in range -95 to +60' % t_prev;
	}
	else if (t_cur < -95.0 || t_cur > 60.0) {
		raise ValueError, 't_cur=%g is not in range -95 to +60' % t_cur;
	}
	//	# for dilay calc delta_t should be greater than 1 day
	else if (delta_t < 1.0) {
		raise ValueError, 'delta_t=%g is less than 1 day' % delta_t;
	}
	else if (soil_heat_cap < 1.0 or soil_heat_cap > 4.5) {
		raise ValueError, 'soil_heat_cap=%g is not in range 1-4.5' % soil_heat_cap;
	}
	else if (delta_z < 0.0 or delta_z > 200.0) {
		raise ValueError, 'delta_z=%g is not in range 0-200 m' % delta_z;
	}
	*/
	//	# Assume an effective soil depth of 0.10 m for a daily calculation as per
	//	# FAO recommendation
	float soil_heat_flux = soil_heat_cap * ((t_cur - t_prev) / delta_t) * delta_z;
		return soil_heat_flux;
}

float delta_sat_vap_pres(float t) {
	/*
		Calculates the slope of the saturation vapour pressure curve at a given
		temperature(t)[kPa degC - 1] based on equation 13 from the FAO paper.For
		use in the Penman - Monteith equation the slope should be calculated using
		mean air temperature.

		Arguments :
		t - air temperature(deg C) (use mean air temp for use in Penman - Monteith)
		
		//# Raise exceptions
	if (t < -95.0 || t > 60.0) {
		raise ValueError, 't=%g is not in range -95 to +60' % t;
	}
	*/
	float	tmp1 = (17.27 * t) / (t + 237.3);

	float	tmp2 = 4098 * (0.6108 * exp(tmp1));
	float delta_es = tmp2 / pow((t + 237.3), 2);
	return delta_es;
}
float ea_from_tmin(float tmin) {
	/*
		Calculates actual vapour pressure, ea[kPa] using equation(48) in
		the FAO paper.This method is to be used where humidity data are
		lacking or are of questionable quality.The method assumes that the
		dewpoint temperature is approximately equal to the minimum temperature
		(T_min), i.e.the air is saturated with water vapour at T_min.
		NOTE: This assumption may not hold in arid / semi - arid areas.
		In these areas it may be better to substract 2 deg C from t_min(see
			Annex 6 in FAO paper).

		Arguments :
		tmin - daily minimum temperature[deg C]
		"""
		//# Raise exception :
	if (tmin < -95.0 || tmin > 60.0) {
		raise ValueError('tmin=%g is not in range -95 to 60 deg C' % tmin);
	}
	*/
	float ea = 0.611 * exp((17.27 * tmin) / (tmin + 237.3));
	return ea;
}
float ea_from_rhmin_rhmax(float e_tmin, float e_tmax, float rh_min, float rh_max) {
	/*
		Calculates actual vapour pressure[kPa] from relative humidity data
		using FAO equation(17).

		Arguments :
		e_tmin - saturation vapour pressure at daily minimum temperature[kPa]
		e_tmax - saturation vapour pressure at daily maximum temperature[kPa]
		rh_min - minimum relative humidity[%]
		rh_max - maximum relative humidity[%]
		"""
		//# Raise exceptions :
	if (rh_min < 0 || rh_min > 100) {
		raise ValueError, 'RH_min=%g is not in range 0-100' % rh_min;
	}
	if (rh_max < 0 || rh_max > 100) {
		raise ValueError, 'RH_max=%g is not in range 0-100' % rh_max;
	}
	*/
	float	tmp1 = e_tmin * (rh_max / 100.0);
	float tmp2 = e_tmax * (rh_min / 100.0);
	float	ea = (tmp1 + tmp2) / 2.0;
	return ea;
}
float ea_from_tdew(float tdew) {
	/*
		Calculates actual vapour pressure, ea[kPa] from the dewpoint temperature
		using equation(14) in the FAO paper.As the dewpoint temperature is the
		temperature to which air needs to be cooled to make it saturated, the
		actual vapour pressure is the saturation vapour pressure at the dewpoint
		temperature.This method is preferable to calculating vapour pressure from
		minimum temperature.

		Arguments:
	tdew - dewpoint temperature[deg C]
	"""
	//# Raise exception :
	if (tdew < -95.0 || tdew > 65.0) {
		//# Are these reasonable bounds ?
		raise ValueError, 'tdew=%g is not in range -95 to +60 deg C' % tdew;
	}
	*/
	float tmp = (17.27 * tdew) / (tdew + 237.3);
	float ea = 0.6108 * exp(tmp);
	return ea;
}
///----------------- SOLAR RADIATION ROUTINES------------------------------------------------
float sol_dec(float doy) {
	/*"""
	Calculates solar declination[rad] from day of the year based on FAO
	equation 24.

	Arguments :
	doy - day of year(between 1 and 366)
	"""
	# Raise exceptions
	if (doy < 1 or doy > 366) :
	raise ValueError, 'doy=%d is not in range 1-366' %doy
	*/
	//	# Calculate solar declination[radians] using FAO eq. 24
	float solar_dec = 0.409 * sin(((2 * PI / 365) * doy - 1.39));
	return solar_dec;
}
float daylight_hours(float sha) {
	/*
	Calculates the number of daylight hours from sunset hour angle
	based on FAO equation 34.

	Arguments :
	sha - sunset hour angle[rad]
	*/
	//# Raise exceptions
	//# TODO: Put in check for sunset hour angle
	float daylight_hours = (24.0 / PI) * sha;
	return daylight_hours;
}
float et_rad(float lat, float sd, float sha, float irl) {
	/*"""
	Calculates daily extraterrestrial radiation('top of the atmosphere
		radiation') [MJ m-2 day-1] using FAO equation 21. If you require a monthly
		mean radiation figure then make sure the solar declination, sunset
		hour angle and inverse relative distance between earth and sun
		provided as function arguments have been calculated using
		the day of the year(doy) that corresponds to the middle of the month.

		Arguments:
lat - latitude[decimal degrees]
sd - solar declination[rad]
sha - sunset hour angle[rad]
irl - inverse relative distance earth - sun[dimensionless]
"""
//# Raise exceptions
//# TODO: raise exceptions for sd and sha
	if (lat < -90.0 || lat > 90.0) {
		raise ValueError, 'latitude=%g is not in range -90 to +90' % lat;
	}
	if (irl < 0.9669 || irl > 1.0331) {
		raise ValueError, 'irl=%g is not in range 0.9669-1.0331' % irl;
	}
	*/
	float		solar_const = 0.0820;  // # Solar constant[MJ m - 2 min - 1]
	float		lat_rad = lat  * (PI / 180);//  # Convert decimal degrees to radians

	//		# Calculate daily extraterrestrial radiation based on FAO equation 21
	float tmp1 = (24 * 60) / PI;
	float tmp2 = sha * sin(lat_rad) * sin(sd);
	float tmp3 = cos(lat_rad) * cos(sd) * sin(sha);
	float et_rad = tmp1 * solar_const * irl * (tmp2 + tmp3);
	return et_rad;
}
float inv_rel_dist_earth_sun(float doy) {
	/*"""
	Calculates the inverse relative distance between earth and sun from
	day of the year using FAO equation 23.

	Arguments :
	doy - day of year[between 1 and 366]
	"""
	# Raise exception
	if (doy < 1 or doy > 366) :
		raise ValueError, 'doy=%d is not in range 1-366' % doy
*/
	float 	inv_rel_dist = 1 + (0.033 * cos((2 * PI / 365)* doy));
	return inv_rel_dist;
}
float clear_sky_rad(int alt, float et_rad) {
	/*
	Calculates clear sky radiation[MJ m - 2 day - 1] based on FAO equation 37
	which is recommended when calibrated Angstrom values are not available.

	Arguments :
	alt - elevation above sea level[m]
	et_rad - extraterrestrial radiation[MJ m - 2 day - 1]

	if (alt < -20 || alt > 8850) {
	raise ValueError, ('altitude=%d is not in range -20 to 8850 m', alt);
	}
	else if(et_rad < 0.0 || et_rad > 50.0)
	{		raise ValueError, ('et_rad=%g is not in range 0-50', et_rad);
	}*/

	float clear_sky_rad = (0.00002 * alt + 0.75) * et_rad;
	return clear_sky_rad;
}
//
float net_in_sol_rad(float sol_rad) {
	/*	"""
	Calculates net incoming solar(also known as shortwave)
	radiation[MJ m - 2 day - 1]
	based on FAO equation 38 for a grass reference crop.This is the net
	shortwave radiation resulting from the balance between incoming and
	reflected solar radiation.The output can be converted to
	equivalent evaporation[mm day - 1] using the equiv_evap() function.

	Arguments:
	sol_rad - (gross)incoming solar radiation[MJ m - 2 day - 1]
	"""
	# Raise exceptions
	# TODO: Put in sensible boundaries for solar radiation
	#if (sol_rad < ?? or sol_rad > ??):
	#    raise ValueError, 'sol_rad=%g is not in range 0-366' %sol_rad
	*/
	float grass_albedo = 0.23;//     # albedo coefficient for grass[dimensionless]
	float net_in_sw_rad = (1 - grass_albedo) * sol_rad;
	return net_in_sw_rad;
}
float net_out_lw_rad(float tmin, float tmax, float sol_rad, float clear_sky_rad, float ea) {
	/*"""
	Calculates net outgoing longwave radiation[MJ m - 2 day - 1] based on
	FAO equation 39. This is the net longwave energy(net energy flux) leaving
	the earth's surface. It is proportional to the absolute temperature of
	the surface raised to the fourth power according to the Stefan - Boltzmann
	law.However, water vapour, clouds, carbon dioxide and dust are absorbers
	and emitters of longwave radiation.This function corrects the Stefan -
	Boltzmann law for humidty(using actual vapor pressure) and cloudiness
	(using solar radiation and clear sky radiation).The concentrations of all
	other absorbers are assumed to be constant.The output can be converted
	to equivalent evapouration[mm day - 1] using the equiv_evap() function.

	Arguments:
	tmin - absolute daily minimum temperature[deg C]
	tmax - absolute daily maximum temperature[deg C]
	sol_rad - solar radiation[MJ m - 2 day - 1]
	clear_sky_rad - clear sky radiation[MJ m - 2 day - 1]
	ea - actual vapour pressure[kPa]
	"""
	# Raise exceptions
	# TODO: raise exceptions for radiation and avp
	if (tmin < -95.0 or tmin > 60.0) :
	raise ValueError, 'tmin=%g is not in range -95 to +60' % tmin
	elif(tmax < -95.0 or tmax > 60.0) :
	raise ValueError, 'tmax=%g is not in range -95 to +60' % tmax

	# Convert temps in deg C to Kelvin
	*/
	float tmin_abs = tmin + 273.15;
	float tmax_abs = tmax + 273.15;

	float sb_const = 0.000000004903;// # Stefan - Boltzmann constant[MJ K - 4 m - 2 day - 1]
	float tmp1 = sb_const * ((pow(tmax_abs, 4) + pow(tmin_abs, 4)) / 2);
	float tmp2 = 0.34 - (0.14 * sqrt(ea));
	float tmp3 = 1.35 * (sol_rad / clear_sky_rad) - 0.35;
	float net_out_lw_rad = tmp1 * tmp2 * tmp3;
	return net_out_lw_rad;
}
//------------------solar radiation calcultion if not available
float sol_rad_from_sun_hours(float dl_hours, float sun_hours, float et_rad) {
	/*    """
	Calculates incoming solar (or shortwave) radiation [MJ m-2 day-1]
	(radiation hitting a horizontal plane after scattering by the atmosphere)
	from relative sunshine duration based on FAO equations 34 and 35.
	If measured radiation data are not available this
	method is preferable to calculating solar radiation from temperature .
	If a monthly mean is required then divide the monthly number
	of sunshine hours by number of days in month and ensure that et_rad and
	daylight hours was calculated using the day of the year that
	corresponds to the middle of the month.

	Arguments:
	dl_hours     - number of daylight hours [hours]
	sun_hours    - sunshine duration [hours]
	et_rad       - extraterrestrial radiation [MJ m-2 day-1]
	"""
	# Raise exceptions
	# TODO: Raise exception for et_rad
	if (sun_hours < 0 or sun_hours > 24):
	raise ValueError, 'sunshine hours=%g is not in range 0-24' % sun_hours
	elif (dl_hours < 0 or dl_hours > 24):
	raise ValueError, 'daylight hours=%g is not in range 0-24' % dl_hours

	# Use default values of regression constants (Angstrom values)
	# recommended by FAO when calibrated values are unavailable.*/
	float a = 0.25;
	float b = 0.50;
	float solar_rad = (b * sun_hours / dl_hours + a) * et_rad;
	return solar_rad;
}
float sol_rad_from_t(float et_rad, float  cs_rad, float  tmin, float  tmax, float  coastal = -999) {
	/*"""
	Calculates incoming solar (or shortwave) radiation (Rs) [MJ m-2 day-1]
	(radiation hitting a horizontal plane after scattering by the atmosphere)
	from min and max temperatures together with
	an empirical adjustment coefficient for 'interior' and
	'coastal' regions. The formula is based on FAO equation 50 which
	is the Hargreaves' radiation formula (Hargreaves and Samani, 1982, 1985).
	This method should be used only when solar radiation or sunshine hours data
	are not available. It is only recommended for locations where it is not
	possible to use radiation data from a regional station (either because
	climate conditions are hetergeneous or data are lacking).
	NOTE: this method is not suitable for island locations
	due to the moderating effects of the surrounding water.

	Arguments:
	et_rad  - extraterrestrial radiation [MJ m-2 day-1]
	cs_rad  - clear sky radiation [MJ m-2 day-1]
	tmin    - daily minimum temperature [deg C]
	tmax    - daily maximum temperature [deg C]
	coastal - True if site is a coastal location, situated on or adjacent to
	coast of a large land mass and where air masses are influence
	by a nearby water body, False if interior location where land
	mass dominates and air masses are not strongly influenced by a
	large water body. -999 indicates no data.
	"""
	# Raise exceptions
	# TODO: raise exceptions for cs_rad
	if (tmin < -95.0 or tmin > 60.0):
	raise ValueError, 'tmin=%g is not in range -95 to +60' % tmin
	elif (tmax < -95.0 or tmax > 60.0):
	raise ValueError, 'tmax=%g is not in range -95 to +60' % tmax

	# determine value of adjustment coefficient [deg C-0.5] for
	# coastal/interior locations*/
	float adj;
	if (coastal == true)
		adj = 0.19;
	else if (coastal == false)
		adj = 0.16;
	else
		//# hedge our bets and give a mean adjustment values and issue a warning
	float 	adj = 0.175;
	//print """WARNING! Location not specified as coastal or interior for
	// calculation of solar radiation. Using defalut adjustment factor."""

	float solar_rad = adj * sqrt(tmax - tmin) * et_rad;

	//# The solar radiation value is constrained (<=) by the clear sky radiation
	if (solar_rad > cs_rad)
		solar_rad = cs_rad;
	return solar_rad;
}
float  sol_rad_island(float et_rad) {
	/* """
	Estimates incoming solar (or shortwave) radiation [MJ m-2 day-1]
	(radiation hitting a horizontal plane after scattering by the atmosphere)
	for an island location using FAO equation 51. An island is defined as a
	land mass with width perpendicular to the coastline <= 20 km. Use this
	method only if radiation data from elsewhere on the island is not
	available. NOTE: This method is only applicable for low altitudes (0-100 m)
	and monthly calculations.

	Arguments:
	et_rad  - extraterrestrial radiation [MJ m-2 day-1]
	"""*/
	float solar_rad = (0.7 * et_rad) - 4.0;
	return solar_rad;
}
//--------------total radiation balance------------------------
float net_rad(float ni_sw_rad, float no_lw_rad) {
	/*"""
	Calculates daily net radiation[MJ m - 2 day - 1] at the crop surface
	based on FAO equations 40 assuming a grass reference crop.
	Net radiation is the difference between the incoming net shortwave(or
	solar) radiation and the outgoing net longwave radiation.Output can be
	converted to equivalent evaporation[mm day - 1] using the equiv_evap()
	function.

	Arguments:
	ni_sw_rad - net incoming shortwave radiation[MJ m - 2 day - 1]
	no_lw_rad - net outgoing longwave radiation[MJ m - 2 day - 1]
	"""
	# Raise exceptions
	# TODO: raise exceptions for radiation arguments
	*/
	float net_rad = ni_sw_rad - no_lw_rad;
	return net_rad;
}


float mean_es(float tmin, float tmax) {
	/*"""
	Calculates mean saturation vapour pressure, es[kPa] using equations(11)
	and (12) in the FAO paper(see references).Mean saturation vapour
	pressure is calculated as the mean of the saturation vapour pressure at
	tmax(maximum temperature) and tmin(minimum temperature).

	Arguments :
	tmin - minimum temperature(deg C)
	tmax - maximum temperature(deg C)
	"""
	# Raise exceptions
	if (tmin < -95.0 or tmin > 60.0) :
		raise ValueError, 'tmin=%g is not in range -95 to +60' % tmin
		elif(tmax < -95.0 or tmax > 60.0) :
		raise ValueError, 'tmax=%g is not in range -95 to +60' % tmax
*/
//		# Saturation vapour pressure at minimum daily temp
	float	tmp1 = (17.27 * tmin) / (tmin + 237.3);
	float	es_tmin = 0.6108 * exp(tmp1);

	//		# Saturation vapour pressure at maximum daily temp
			tmp1 = (17.27 * tmax) / (tmax + 237.3);
	float es_tmax = 0.6108 * exp(tmp1);
	float mean_es = (es_tmin + es_tmax) / 2.0;
	return mean_es;
}
float monthly_soil_heat_flux(float t_month_prev, float t_month_next) {
	/*		"""
			Estimates the monthly soil heat flux(Gmonth)[MJ m - 2 day - 1]
			assuming a grass crop from the mean
			air temperature of the previous month and the next month based on FAO
			equation(43).If the air temperature of the next month is not known use
			function monthly_soil_heat_flux2().The resluting heat flux can be
			converted to equivalent evaporation[mm day - 1] using the equiv_evap()
			function.

			Arguments:
t_month_prev - mean air temperature of previous month[deg C]
t_month2_next - mean air temperature of next month[deg C]
"""
# Raise exceptions
if (t_month_prev < -95.0 or t_month_prev > 60.0) :
	raise ValueError, 't_month_prev=%g is not in range -95 to +60' % t_month_prev
	elif(t_month_next < -95.0 or t_month_next > 60.0) :
	raise ValueError, 't_month_next=%g is not in range -95 to +60' % t_month_next
*/
	float soil_heat_flux = 0.07 * (t_month_next - t_month_prev);
	return soil_heat_flux;
}
float monthly_soil_heat_flux2(float t_month_prev, float t_month_cur) {
	/*"""
	Estimates the monthly soil heat flux(Gmonth)[MJ m - 2 day - 1]
	assuming a grass crop from the mean
	air temperature of the previous and current month based on FAO
	equation(44).If the air temperature of the next month is available use
	monthly_soil_heat_flux() function instead.The resluting heat flux can be
	converted to equivalent evaporation[mm day - 1] using the equiv_evap()
	function.

	Arguments:
t_month_prev - mean air temperature of previous month[deg C]
t_month2_cur - mean air temperature of current month[deg C]
"""
# Raise exceptions
if (t_month_prev < -95.0 or t_month_prev > 60.0) :
	raise ValueError, 't_month_prev=%g is not in range -95 to +60' % t_month_prev
	elif(t_month_cur < -95.0 or t_month_cur > 60.0) :
	raise ValueError, 't_month_cur=%g is not in range -95 to +60' % t_month_cur
*/
	float soil_heat_flux = 0.14 * (t_month_cur - t_month_prev);
	return soil_heat_flux;
}

float penman_monteith_ETo(float Rn, float t, float ws, float es, float ea, float delta_es, float psy, float shf = 0.0) {
	/*	"""
		Calculates the evapotransporation(ETo)[mm day - 1] from a hypothetical
		grass reference surface using the FAO Penman - Monteith equation(equation 6).

		Arguments :
		Rn - net radiation at crop surface[MJ m - 2 day - 1] net_rad<-(net_in_sol_rad,net_out_lw_rad)
		t - air temperature at 2 m height[deg C]
		ws - wind speed at 2 m height[m s - 1].If not measured at 2m,convert using wind_speed_at_2m()
		es - saturation vapour pressure[kPa]
		ea - actual vapour pressure[kPa]
		delta_es - slope of vapour pressure curve[kPa  deg C]
		psy - psychrometric constant[kPa deg C]
		shf - soil heat flux(MJ m - 2 day - 1] (default = 0, fine for daily time step)
			"""
			# TODO: raise exceptions for radiation and avp / svp etc.
			if (t < -95.0 or t > 60.0) :
				raise ValueError, 't=%g is not in range -95 to +60' % t
				elif(ws < 0.0 or ws > 150.0) :
				raise ValueError, 'ws=%g is not in range 0-150' % ws
	*/
	//		# Convert t in deg C to deg Kelvin
	 t += 273.15;
		//		# Calculate evapotranspiration(ET0)
	float	a1 = 0.408 * (Rn - shf) * delta_es / (delta_es + (psy * (1 + 0.34 * ws)));
	float a2 = 900 * ws / t * (es - ea) * psy / (delta_es + (psy * (1 + 0.34 * ws)));
	float ETo = a1 + a2;
	return ETo;
}

float psy_const(float atmos_pres) {
	/*"""
	Calculates the psychrometric constant(kPa degC - 1) using equation(8)
	in the FAO paper(see references below) page 95. This method assumes that
	the air is saturated with water vapour at T_min.This assumption may not
	hold in arid areas.

	Arguments :
	atmos_pres - atmospheric pressure[kPa]
	"""
	# TODO : raise exception if atmos_press outside sensible bounds
	*/
	return 0.000665 * atmos_pres;
}
float psy_const_of_psychrometer(float psychrometer, float atmos_pres) {
	/*"""
	Calculates the psychrometric constant[kPa deg C - 1] for different
	types of psychrometer at a given atmospheric pressure using FAO equation
	16.

	Arguments :
	psychrometer - integer between 1 and 3 which denotes type of psychrometer
	- 1 = ventilated(Asmann or aspirated type) psychrometer with
	an air movement of approx. 5 m s - 1
	- 2 = natural ventilated psychrometer with an air movement
	of approx. 1 m s - 1
	- 3 = non ventilated psychrometer installed indoors
	atmos_pres - atmospheric pressure[kPa]
	"""
	# TODO: raise exception if atmos_press outside sensible bounds
	if (psychrometer < 1 or psychrometer > 3) :
		raise ValueError, 'psychrometer=%d not in range 1-3' % psychrometer

		# Assign values to coefficient depending on type of ventilation of the
		# wet bulb*/
	float psy_coeff;
	if (psychrometer == 1)
		psy_coeff = 0.000662;
	else if (psychrometer == 2)
		psy_coeff = 0.000800;
	else if (psychrometer == 3)
		psy_coeff = 0.001200;

	float	pys_const = psy_coeff * atmos_pres;
	return pys_const;
}

float rad2equiv_evap(float energy) {
	/*"""
	Converts radiation in MJ m - 2 day - 1 to the equivalent evaporation in
	mm day - 1 assuming a grass reference crop using FAO equation 20.
	Energy is converted to equivalent evaporation using a conversion
	factor equal to the inverse of the latent heat of vapourisation
	(1 / lambda = 0.408).

	Arguments:
energy - energy e.g.radiation, heat flux[MJ m - 2 day - 1]
"""
# Determine the equivalent evaporation[mm day - 1]*/
	float equiv_evap = 0.408 * energy;
	return equiv_evap;
}

float  rh_from_ea_es(float ea, float es) {
	/*"""
	Calculates relative humidity as the ratio of actual vapour pressure
	to saturation vapour pressure at the same temperature(see FAO paper
		p. 67).

	ea - actual vapour pressure[units don't matter as long as same as es]
	es - saturated vapour pressure[units don't matter as long as same as ea]
	"""*/
	return 100.0 * ea / es;
}


float  wind_speed_2m(float meas_ws, float  z) {
	/*"""
	Converts wind speeds measured at different heights above the soil
	surface to wind speed at 2 m above the surface, assuming a short grass
	surface. Formula based on FAO equation 47.

	Arguments:
	meas_ws - measured wind speed [m s-1]
	z       - height of wind measurement above ground surface [m]
	"""
	# Raise exceptions
	if (meas_ws < 0.0 or meas_ws > 150.0):
		raise ValueError, 'meas_ws=%g is not in range 0-150 m s-1' % meas_ws
	elif (z < 0.0 or z > 100.0):
		raise ValueError, 'z=%g is not in range 0-100 m' % z
*/
	float   tmp1 = (67.8 * z) - 5.42;
	float   ws2m = meas_ws * (4.87 / log(tmp1));
	return ws2m;
}

float ETo_hourly(float rad, float t, float ws, float es, float ea,  float delta_es, float psy) {
	//calculating Rn-G net radiation-soil heat flux per hour
	float Rn = 0.0036*(0.76*rad - 38.5);   //radiation in mm/h
	float g;
	if (rad > 1)
		g = 0.1*Rn;				//during day
	else
		g = 0.5*Rn;				//during nigth

	float term1 = psy * 37 / (t + 273.2)*ws*(es - ea);
	float ETo = (0.408*delta_es*(Rn - g) + term1) / (delta_es + psy*(1 + 0.34*ws));
	Serial.printf(" ET %d term %d Rn %d g %d ", ETo, term1, Rn, g);
	return ETo;
}