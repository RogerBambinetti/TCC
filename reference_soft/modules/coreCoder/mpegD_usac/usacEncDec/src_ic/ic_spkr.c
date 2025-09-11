#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "ic_defs.h"

int getSpkrLbl ( int * lbl, int * nspkr, int cicpidx )
{
	int		ret=1;

	switch ( cicpidx )
	{
	case 1:
		*nspkr	= 1;
		lbl[0]	= CHN_M_000;
		break;
	case 2:
		*nspkr	= 2;
		lbl[0]	= CHN_M_L030;
		lbl[1]	= CHN_M_R030;
		break;
	case 3:
		*nspkr	= 3;
		lbl[0]	= CHN_M_L030;
		lbl[1]	= CHN_M_R030;
		lbl[2]	= CHN_M_000;
		break;
	case 4:
		*nspkr	= 4;
		lbl[0]	= CHN_M_L030;
		lbl[1]	= CHN_M_R030;
		lbl[2]	= CHN_M_000;
		lbl[3]	= CHN_M_180;
		break;
	case 5:
		*nspkr	= 5;
		lbl[0]	= CHN_M_L030;
		lbl[1]	= CHN_M_R030;
		lbl[2]	= CHN_M_000;
		lbl[3]	= CHN_M_L110;
		lbl[4]	= CHN_M_R110;
		break;
	case 6:
		*nspkr	= 6;
		lbl[0]	= CHN_M_L030;
		lbl[1]	= CHN_M_R030;
		lbl[2]	= CHN_M_000;
		lbl[3]	= CHN_LFE1;
		lbl[4]	= CHN_M_L110;
		lbl[5]	= CHN_M_R110;
		break;
	case 7:
		*nspkr	= 8;
		lbl[0]	= CHN_M_L030;
		lbl[1]	= CHN_M_R030;
		lbl[2]	= CHN_M_000;
		lbl[3]	= CHN_LFE1;
		lbl[4]	= CHN_M_L110;
		lbl[5]	= CHN_M_R110;
		lbl[6]	= CHN_M_L060;
		lbl[7]	= CHN_M_R060;
		break;
	/*case 8:		not available*/
	case 9:
		*nspkr	= 3;
		lbl[0]	= CHN_M_L030;
		lbl[1]	= CHN_M_R030;
		lbl[2]	= CHN_M_180;
		break;
	case 10:
		*nspkr	= 4;
		lbl[0]	= CHN_M_L030;
		lbl[1]	= CHN_M_R030;
		lbl[2]	= CHN_M_L110;
		lbl[3]	= CHN_M_R110;
		break;
	case 11:
		*nspkr	= 7;
		lbl[0]	= CHN_M_L030;
		lbl[1]	= CHN_M_R030;
		lbl[2]	= CHN_M_000;
		lbl[3]	= CHN_LFE1;
		lbl[4]	= CHN_M_L110;
		lbl[5]	= CHN_M_R110;
		lbl[6]	= CHN_M_180;
		break;
	case 12:
		*nspkr	= 8;
		lbl[0]	= CHN_M_L030;
		lbl[1]	= CHN_M_R030;
		lbl[2]	= CHN_M_000;
		lbl[3]	= CHN_LFE1;
		lbl[4]	= CHN_M_L110;
		lbl[5]	= CHN_M_R110;
		lbl[6]	= CHN_M_L135;
		lbl[7]	= CHN_M_R135;
		break;
	case 13:
		*nspkr	= 24;
		lbl[0]	= CHN_M_L060;
		lbl[1]	= CHN_M_R060;
		lbl[2]	= CHN_M_000;
		lbl[3]	= CHN_LFE2;
		lbl[4]	= CHN_M_L135;
		lbl[5]	= CHN_M_R135;
		lbl[6]	= CHN_M_L030;
		lbl[7]	= CHN_M_R030;
		lbl[8]	= CHN_M_180;
		lbl[9]	= CHN_LFE3;
		lbl[10]	= CHN_M_L090;
		lbl[11]	= CHN_M_R090;
		lbl[12]	= CHN_U_L045;
		lbl[13]	= CHN_U_R045;
		lbl[14]	= CHN_U_000;
		lbl[15]	= CHN_T_000;
		lbl[16]	= CHN_U_L135;
		lbl[17]	= CHN_U_R135;
		lbl[18]	= CHN_U_L090;
		lbl[19]	= CHN_U_R090;
		lbl[20]	= CHN_U_180;
		lbl[21]	= CHN_L_000;
		lbl[22]	= CHN_L_L045;
		lbl[23]	= CHN_L_R045;
		break;
	case 14:
		*nspkr	= 8;
		lbl[0]	= CHN_M_L030;
		lbl[1]	= CHN_M_R030;
		lbl[2]	= CHN_M_000;
		lbl[3]	= CHN_LFE1;
		lbl[4]	= CHN_M_L110;
		lbl[5]	= CHN_M_R110;
		lbl[6]	= CHN_U_L030;
		lbl[7]	= CHN_U_R030;
		break;
	case 15:
		*nspkr	= 12;
		lbl[0]	= CHN_M_L030;
		lbl[1]	= CHN_M_R030;
		lbl[2]	= CHN_M_000;
		lbl[3]	= CHN_LFE2;
		lbl[4]	= CHN_M_L135;
		lbl[5]	= CHN_M_R135;
		lbl[6]	= CHN_LFE3;
		lbl[7]	= CHN_M_L090;
		lbl[8]	= CHN_M_R090;
		lbl[9]	= CHN_U_L045;
		lbl[10]	= CHN_U_R045;
		lbl[11]	= CHN_U_180;
		break;
	case 16:
		*nspkr	= 10;
		lbl[0]	= CHN_M_L030;
		lbl[1]	= CHN_M_R030;
		lbl[2]	= CHN_M_000;
		lbl[3]	= CHN_LFE1;
		lbl[4]	= CHN_M_L110;
		lbl[5]	= CHN_M_R110;
		lbl[6]	= CHN_U_L030;
		lbl[7]	= CHN_U_R030;
		lbl[8]	= CHN_U_L110;
		lbl[9]	= CHN_U_R110;
		break;
	case 17:
		*nspkr	= 12;
		lbl[0]	= CHN_M_L030;
		lbl[1]	= CHN_M_R030;
		lbl[2]	= CHN_M_000;
		lbl[3]	= CHN_LFE1;
		lbl[4]	= CHN_M_L110;
		lbl[5]	= CHN_M_R110;
		lbl[6]	= CHN_U_L030;
		lbl[7]	= CHN_U_R030;
		lbl[8]	= CHN_U_000;
		lbl[9]	= CHN_U_L110;
		lbl[10]	= CHN_U_R110;
		lbl[11]	= CHN_T_000;
		break;
	case 18:
		*nspkr	= 14;
		lbl[0]	= CHN_M_L030;
		lbl[1]	= CHN_M_R030;
		lbl[2]	= CHN_M_000;
		lbl[3]	= CHN_LFE1;
		lbl[4]	= CHN_M_L110;
		lbl[5]	= CHN_M_R110;
		lbl[6]	= CHN_M_L150;
		lbl[7]	= CHN_M_R150;
		lbl[8]	= CHN_U_L030;
		lbl[9]	= CHN_U_R030;
		lbl[10]	= CHN_U_000;
		lbl[11]	= CHN_U_L110;
		lbl[12]	= CHN_U_R110;
		lbl[13]	= CHN_T_000;
		break;
	case 19:
		*nspkr	= 12;
		lbl[0]	= CHN_M_L030;
		lbl[1]	= CHN_M_R030;
		lbl[2]	= CHN_M_000;
		lbl[3]	= CHN_LFE1;
		lbl[4]	= CHN_M_L135;
		lbl[5]	= CHN_M_R135;
		lbl[6]	= CHN_M_L090;
		lbl[7]	= CHN_M_R090;
		lbl[8]	= CHN_U_L030;
		lbl[9]	= CHN_U_R030;
		lbl[10]	= CHN_U_L135;
		lbl[11]	= CHN_U_R135;
		break;
	case 20:
		*nspkr	= 14;
		lbl[0]	= CHN_M_L030;
		lbl[1]	= CHN_M_R030;
		lbl[2]	= CHN_M_000;
		lbl[3]	= CHN_LFE1;
		lbl[4]	= CHN_M_L135;
		lbl[5]	= CHN_M_R135;
		lbl[6]	= CHN_M_L090;
		lbl[7]	= CHN_M_R090;
		lbl[8]	= CHN_U_L045;
		lbl[9]	= CHN_U_R045;
		lbl[10]	= CHN_U_L135;
		lbl[11]	= CHN_U_R135;
		lbl[12]	= CHN_M_LSCR;
		lbl[13]	= CHN_M_RSCR;
		break;	
	default:
		ret=0;
		break;
	}

	return ret;
}