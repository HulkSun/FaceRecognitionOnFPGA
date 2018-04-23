#include <string>
#include <bitset>
#include <stdlib.h>
#include <iostream>
#include <fstream>

#include "types.h"
//#include "linscan.h"

#include "jni.h"
#include "com_neptune_api_MIH.h"

using namespace std;

int K = 0;
int B_over_8 = 0;

static jclass java_util_ArrayList;

static Linscan* linscan = NULL;

/*
 * Class:     com_neptune_api_MIH
 * Method:    init
 * Signature: (IIII)V
 */
JNIEXPORT void JNICALL Java_com_neptune_api_MIH_init
  (JNIEnv *env, jclass thisz, jint b, jint k, jint d, jint dayn)
{
    if(linscan != NULL){
		delete linscan;
	}
    linscan = new Linscan(b, k, d, dayn);

    K = k;
    B_over_8 = b/8;

	jclass local_java_util_ArrayList = env->FindClass("java/util/ArrayList");
	java_util_ArrayList = (jclass)env->NewGlobalRef(local_java_util_ArrayList);
	env->DeleteLocalRef(local_java_util_ArrayList);
}


/*
 * Class:     com_neptune_api_MIH
 * Method:    appendCodes
 * Signature: ([BI)Z
 */
JNIEXPORT jboolean JNICALL Java_com_neptune_api_MIH_appendCodes
  (JNIEnv *env, jclass thisz, jbyteArray codes, jint day){

    UINT8* u_hash = (UINT8*)malloc(sizeof(UINT8) * B_over_8);
	
	if(u_hash == NULL) return false;
	
	env->GetByteArrayRegion(codes, 0, B_over_8, (jbyte*)u_hash);

    linscan->add_codes(u_hash, day);
	
	return true;
}

/*
 * Class:     com_neptune_api_MIH
 * Method:    getCandidates
 * Signature: ([BZI)Ljava/util/List;
 */
JNIEXPORT jobject JNICALL Java_com_neptune_api_MIH_getCandidates
  (JNIEnv *env, jclass thisz, jbyteArray query_hash, jboolean isadd, jint day){

	//std::ofstream out("linscan_interface.log");
	static jmethodID ArrayList_construct = env->GetMethodID(java_util_ArrayList, "<init>", "()V");
	static jmethodID java_util_ArrayList_add  = env->GetMethodID(java_util_ArrayList, "add", "(Ljava/lang/Object;)Z");

    UINT8* u_hash = (UINT8*)malloc(sizeof(UINT8) * B_over_8);

	if(u_hash == NULL) return NULL;
	//out << "linscan_interface u_hash malloc done.\n";

	env->GetByteArrayRegion(query_hash, 0, B_over_8, (jbyte*)u_hash);

    UINT8** candies = (UINT8 **) malloc(sizeof(UINT8*)*K);
	
	if(candies == NULL) return NULL;

	//out << "linscan_interface candies malloc done.\n";
    linscan->query(u_hash, candies, day);
	//out << "linscan_interface query done.\n";
    if(isadd){
	   	linscan->add_codes(u_hash, day);
	}else{
		free(u_hash);
	}

	//out << "linscan_interface add_codes done.\n";
    jobject result = env->NewObject(java_util_ArrayList, ArrayList_construct);
	
	if(result == NULL) return NULL;

    for(int i=0;i<K && candies[i] != NULL;i++){
		jbyteArray byte_cand = env->NewByteArray(B_over_8);
		env->SetByteArrayRegion(byte_cand, 0, B_over_8, (jbyte*)candies[i]);
		env->CallBooleanMethod(result, java_util_ArrayList_add, byte_cand);
		env->DeleteLocalRef(byte_cand);
    }
	
	free(candies);

	//out << "linscan_interface get_result done.\n";
	//out.close();
    return result;
}



/*
 * Class:     com_neptune_api_MIH
 * Method:    destory
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_neptune_api_MIH_destory
  (JNIEnv *env, jclass thisz){
    if(linscan != NULL){
        delete linscan;
		linscan = NULL;
    }
}

/*
 * Class:     com_neptune_api_MIH
 * Method:    rotateNewDayCodes
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_neptune_api_MIH_rotateNewDayCodes
  (JNIEnv *env, jclass thisz)
{
    linscan->move_codes(1, 0, linscan->codes_size() - 1);
    linscan->clear_codes(0);
}

