//#include "header.h"
#ifndef INTERPOLATION_H
#define INTERPOLATION_H

#include "cfg_types.h"
#include "lib_bitmanip.h"

//#define AMFM_APP_NORMAL_RSSI                    (Tu16)(16+(20*2))
//#define AMFM_APP_NORMAL_WAM                     (Tu16)((255*25)/100)
//#define AMFM_APP_NORMAL_USN                     (Tu16)((255*20)/100)


//#define AMFM_APP_REGIONAL_RSSI                   (Tu16)(16+(18*2))
//#define AMFM_APP_REGIONAL_WAM                    (Tu16)((255*33)/100)
//#define AMFM_APP_REGIONAL_USN                    (Tu16)((255*33)/100)

#define INTERPOLATION_OFS_ENABLE	1
#define INTERPOLATION_OFS_DISABLE	0

#define INTERPOLATION_OFS_THRESHOLD  20

#define TUN_PH_FS_MIN   (tun_ph_qual_fs_steps[0]) /* dBµV */
#define TUN_PH_FS_MAX   (tun_ph_qual_fs_steps[TUN_PH_FS_STEPS_SIZE-1u]) /* dBµV */
#define TUN_PH_FS_STEPS {0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75}
#define TUN_PH_FS_STEPS_SIZE 16u

#define TUN_PH_USN_MIN  (tun_ph_qual_usn_steps[0])
#define TUN_PH_USN_MAX  (tun_ph_qual_usn_steps[TUN_PH_USN_STEPS_SIZE-1u])
#define TUN_PH_USN_STEPS {0, 5, 15, 25, 35}
#define TUN_PH_USN_STEPS_SIZE 5u

#define TUN_PH_WAM_MIN (tun_ph_qual_wam_steps[0])
#define TUN_PH_WAM_MAX (tun_ph_qual_wam_steps[TUN_PH_WAM_STEPS_SIZE-1u])
#define TUN_PH_WAM_STEPS {0, 8, 13, 18, 25}
#define TUN_PH_WAM_STEPS_SIZE 5u

#define TUN_QUAL_ERROR 0


//Tu8 Fm_quality_check(Ts8 fs, Ts8 adj, Ts8 mp,Tu8* q_fs, Tu8* q_no_mp);

Tu32 QUAL_CalcFmQual(Ts32 s32_BBFieldStrength, Tu32 u32_UltrasonicNoise, Tu32 u32_Multipath ,Tu32* q_BBFieldStrength, Tu32* q_no_Multipath,Tu32 u32_FrequencyOffset);
Tu32 QUAL_CalcQuality(Ts32 s32_BBFieldStrength, Tu32 u32_UltrasonicNoise, Tu32 u32_Multipath ,Tu32* q_BBFieldStrength, Tu32* q_no_Multipath);

#endif /* End of INTERPOLATION_H*/