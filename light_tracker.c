#include <stdio.h>


int main(){
// 조도 센서 8개가 읽어오는 값
int top_left = 0;
int top_middle = 0;
int top_right =0;
int middle_left = 0;
int middle_right =0;
int bottom_left =0;
int bottom_right=0;
int bottom_middle = 0;

// 축
int Axis_h =0; // 수직
int Axis_v =0; // 수평

// 평균값
int avg_top =0;
int avg_bottom =0;
int avg_left =0;
int avg_right =0;


// 값 비교
avg_top = (top_left+top_middle+top_right)/3;
avg_bottom = (bottom_left+bottom_middle+bottom_right)/3;
avg_left = (top_left+middle_left+bottom_left)/3;
avg_right = (top_right+middle_right+bottom_right)/3;

while(1){
    
Axis_h = avg_top - avg_bottom;
if(Axis_h >0){ //top이 더밝을때
    // 판넬을 top으로 이동
}else if(Axis_h<0){// bottom이 더 밝을때
    // 판넬을 bottom으로 이동
}else{ // avg_top = avg_bottom일때는 이동x
    // 이동 x
}

Axis_v = avg_left - avg_right;
if(Axis_v > 0){ // left가 더 밝을때
    // 판넬을 left로 이동
}else if(Axis_v < 0 ){ // right가 더 밝을 때
    // 판넬을 right로 이동
}else{ // avg_left = avg_right 일때
    // 이동 x
}

}


    return 0;
}