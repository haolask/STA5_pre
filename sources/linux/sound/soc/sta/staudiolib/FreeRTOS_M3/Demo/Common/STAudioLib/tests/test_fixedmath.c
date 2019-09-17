/*
 * STAudioLib - test_fixedmath.c
 *
 * Created on 2013/04/22 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*

*/

#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

#if 1

#if STA_NO_FLOAT == 0
#error "test_fixedmath.c" should be compiled with STA_NO_FLOAT=1
#else

#include "staudio_test.h"

#include "mathFixed.h"
#include "math.h"

//#include "audioTypes.h" //tBiquad

typedef signed long long s64;


#define F(a)			FRAC_(a)

#define FLT_(a)			((float)(a) / FRAC_1)

#define PI				3.14159265358979323846

//Biquad Gain "ooStep" (1/step)
#define BGS				4


//---------------------------------------------------------
// test float / double sin, cos...
//---------------------------------------------------------

static void test_float_math(void)
{
	double d1,d2;
	float  f1,f2;

	d1 = cos(PI);
	f1 = cosf((float)PI);

	d1 = log10(100);
	f1 = log10f(100);

	d1 = log(2);
	f1 = logf(2);

	d1 = exp(1.2f);
	d2 = expf(1.2f);
	f1 = exp(1.2f);
	f2 = expf(1.2f);

	d1 = pow(1.5, 2);  //=2.25
	f1 = powf(1.5, 2);

}

//---------------------------------------------------------
// test F_SIN, F_COS
//---------------------------------------------------------

static void test_SIN_COS(void)
{
	#define N  9
//	const float a[N] = {(0), (PI/4), (PI/2), (PI*3/4), (PI*4/4), (PI*5/4), (PI*6/4), (PI*7/4), (PI*2)};
	const float a[N] = {(0),-(PI/4),-(PI/2),-(PI*3/4),-(PI*4/4),-(PI*5/4),-(PI*6/4),-(PI*7/4),-(PI*2)};

	s32   cx[N],  sx[N];
	float cxf[N], sxf[N];
	float cf[N],  sf[N];
//	float cf2[N],  sf2[N];
	int i;

	FORi(N) {
		cx[i]  = F_COS(F(a[i]));
		sx[i]  = F_SIN(F(a[i]));

		cxf[i] = FLT_(cx[i]);
		sxf[i] = FLT_(sx[i]);

		cf[i]  = cosf(a[i]);
		sf[i]  = sinf(a[i]);

//		cf2[i]  = cos(a[i]);  //same result as with cosf
//		sf2[i]  = sin(a[i]);
	}

	//CONCLUSION:
	//F_COS and F_SIN acceptable from -PI*5/4 to +PI*5/4, let's say -PI to PI

	SLEEP(100);
	#undef N
}

//---------------------------------------------------------
// test omega = 2.0f*PI * fc / fs
//---------------------------------------------------------

static void test_cos_omega(void)
{
	const int N = 6;
	const s32 fc[6] = {20, 4000, 8000, 12000, 16000, 20000};
	s32 wref[6], w1[6], w2[6];
	int i;

	FORi(N){
		wref[i] = FRAC_(2.0f*PI * fc[i] / 48000); 	//REF (same as MATLAB)

		w1[i] = FRAC_(PI*2 / 48000) * fc[i];  		//lose some precision
//		w2[i] = FRAC_(PI*2) / 48000 * fc[i];		//exactly the ssame
//		w2[i] = fc[i] / 48000 * FRAC_(PI*2);		//WRONG
		w2[i] = (s64) FRAC_(PI*2) * fc[i] / 48000;	//GOOD
	}
}

//---------------------------------------------------------
// test BIQ_BandShelv
//---------------------------------------------------------

typedef struct sBiquadf {
	STA_FilterType	m_type;
	void 			(*m_pUpdateCoefs)(struct sBiquadf* bq);
	//input params
	float 			m_G;		//Gain dB
	float 			m_g;		//Linear gain = 10^(G/20.0)
	float 			m_q;		//Quality factor [0.5:6]
	float 			m_fc;		//Cut off freq [Hz]
	float 			m_fs;		//Sampling freq [Hz] (ex. 45600)
	float			m_fn;		//Normalised freq = fc/fs
	float			m_K;		//tmp value = 1/tan(fn * PI)
	float			m_cosw0;	//= cos(2*PI*fn)
	float			m_sinw0;	//= sin(2*PI*fn)
	//output coefs
	float 			m_a[3];
	float 			m_b[3];
	float   		m_bshift;
	float   		m_outshift;
	//flags
	u32  			m_dirtyBiquad:1; //set when one input param is modified
} tBiquadf;


//Hold the params and coef in float, before conversion to DSP's fraction (1.23 bits)
typedef struct sBiquad {
	STA_FilterType	m_type;
	void 			(*m_pUpdateCoefs)(struct sBiquad* bq);
	//input params
	s32 			m_G;		//Gain dB
	s32 			m_g;		//Linear gain = 10^(G/20.0)
	s32 			m_q;		//Quality factor [0.5:6]
	s32 			m_fc;		//Cut off freq [Hz]
	s32 			m_fs;		//Sampling freq [Hz] (ex. 45600)
	s32				m_fn;		//Normalised freq = fc/fs
	s32				m_K;		//tmp value = 1/tan(fn * PI)
	s32				m_cosw0;	//= cos(2*PI*fn)
	s32				m_sinw0;	//= sin(2*PI*fn)
	//output coefs
	s32 			m_a[3];
	s32 			m_b[3];
	u16   			m_bshift;
	u16   			m_outshift;
	//flags
	u32  			m_dirtyBiquad:1; //set when one input param is modified
} tBiquad;


static void BIQ_Initf(tBiquadf* bq, STA_FilterType type, s32 G, s32 Q, s32 fc)
{
	float* b = bq->m_b;
	float* a = bq->m_a;

	bq->m_G = (float) G / BGS;			//int to float
	bq->m_g = pow(10, bq->m_G / 20);	//g = 10^(G/20.0)

	bq->m_q = 0.1f * Q; //int to float

	bq->m_fc = (float) fc;
	bq->m_fs = (float) 48000;
	bq->m_fn = (float) fc / 48000;
	bq->m_K  = 1.0f / tan(bq->m_fn * M_PI);

	b[0] = a[0] = 1;
	b[1] = b[2] = a[1] = a[2] = 0;

	bq->m_bshift = bq->m_outshift = 0;
}

//REF (float)
static void BIQ_BandShelvf(tBiquadf* bq)//, float fn, float g, float q)
{
	float  g = bq->m_g;
	float  q = bq->m_q;
	float* b = bq->m_b;
	float* a = bq->m_a;
	float  k = bq->m_fn; // = Fc/Fs;
	float ang = bq->m_fn * 2 * M_PI;
	float  k1, k2, sina;

//	if (!bq->m_dirtyBiquad) return; //nothing to do

	//Q correction
	if (k >= 0.04386) {  // fc >= 2 kHz if fs = 45600  TODO: update for 48000
		const float aqc = -3.119513761;
		const float bqc = -0.5179761063;
		const float cqc =  1.021853509;

		const float aq15 =  0.006801984152;
		const float bq15 = -0.06130098505;
		const float cq15 =  0.6045898018;

		float qcomp    = MIN(4, q);
		float qoff15   = aq15*qcomp*qcomp + bq15*qcomp + cq15 - 0.5055;

		float tmp      = 1.0f - 3.652f* ABS(0.32894 - k);
		float qoffcorr = MAX(0, tmp);

		float qoffset  = qoffcorr * qoff15;

		q = (aqc*k*k + bqc * k + cqc + qoffset) * q;
	}


	//a, b
	if (g < 1)
		q *= g;
    k1 = - cosf(ang);
    sina = sinf(ang);
    k2 = (2*q - sina) / (2*q + sina);

	b[0] = (1 + k2 + g * (1 - k2)) * 0.5;
	b[1] = k1 * (1 + k2);
	b[2] = (1 + k2 - g * (1 - k2)) * 0.5;
	a[0] = 1.0;
	a[1] = b[1];
	a[2] = k2;

//	BIQ_GetAndApplyBshift(bq);
}

extern void BIQ_Init(tBiquad* bq, STA_FilterType type, s32 G, s32 Q, s32 fc);

static void BIQ_BandShelvx(tBiquad* bq)//, float fn, float g, float q)
{
	f32  g = bq->m_g;
	f32  q = bq->m_q;
	f32  k = bq->m_fn; // = Fc/Fs;
	f32* b = bq->m_b;
	f32* a = bq->m_a;
	f32  k1, k2, k3;

//	if (!bq->m_dirtyBiquad) return; //nothing to do

	//Q correction
	if (k >= FRAC_(0.04386)) {  // fc >= 2 kHz if fs = 45600  TODO: update for 48000
		const f32 aqc = FRAC_(-3.119513761);
		const f32 bqc = FRAC_(-0.5179761063);
		const f32 cqc = FRAC_( 1.021853509);

		const f32 aq15 = FRAC_( 0.006801984152);
		const f32 bq15 = FRAC_(-0.06130098505);
		const f32 cq15 = FRAC_( 0.6045898018);

		f32 qcomp    = MIN(FRAC_(4), q);
		f32 qcomp2 	 = F_MUL(qcomp,qcomp);
		f32 qoff15   = F_MUL(aq15, qcomp2) + F_MUL(bq15,qcomp) + cq15 - FRAC_(0.5055);

		f32 tmp      = FRAC_1 - F_MUL(FRAC_(3.652f), ABS(FRAC_(0.32894) - k));
		f32 qoffcorr = MAX (0, tmp);

		f32 qoffset  = F_MUL(qoffcorr, qoff15);

		f32 kk = F_MUL(k,k);

		tmp = F_MUL(aqc, kk) + F_MUL(bqc, k) + cqc + qoffset;

		q = F_MUL(tmp, q);
	}

	//a, b
	if (g < FRAC_1)
		q = F_MUL(q, g);

	k1 = -bq->m_cosw0;
	k2 = F_DIV(2*q - bq->m_sinw0, 2*q + bq->m_sinw0);
	k3 = F_MUL(g, FRAC_1 - k2);

	b[0] = (FRAC_1 + k2 + k3) >> 1;
	b[1] = F_MUL(k1, FRAC_1 + k2);
	b[2] = (FRAC_1 + k2 - k3) >> 1;
	a[0] = FRAC_1;
	a[1] = b[1];
	a[2] = k2;

//	BIQ_GetAndApplyBshift(bq);

}

static void test_bandshelv(void)
{
	tBiquadf biqf, biqxf;
	tBiquad  biqx;
	int i;

	BIQ_Initf(&biqf, STA_BIQUAD_BAND_BOOST_2, 10*BGS, 30, 3000); //try with different freqs
	BIQ_Init (&biqx, STA_BIQUAD_BAND_BOOST_2, 10*BGS, 30, 3000);

	BIQ_BandShelvf(&biqf);
	BIQ_BandShelvx(&biqx);

	//CHECK
	biqxf.m_g  = FLT_(biqx.m_g);
	biqxf.m_q  = FLT_(biqx.m_q);
	biqxf.m_fn = FLT_(biqx.m_fn);
	FORi(3) {
	biqxf.m_a[i] = FLT_(biqx.m_a[i]);
	biqxf.m_b[i] = FLT_(biqx.m_b[i]);
	}

}


//---------------------------------------------------------
//
//---------------------------------------------------------
void TestFixedMath(void)
{
	test_float_math();
//	test_SIN_COS();
//	test_cos_omega();
	test_bandshelv();

	while (1)
		SLEEP(100);
}

#endif //STA_NO_FLOAT=1
#endif //0
