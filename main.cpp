#include <iostream>
#include <array>
#include <chrono>
#include <windows.h>

// 1. 센서 데이터 한 건을 담을 구조체
struct SensorData {
    int id;                                             // 데이터 번호
    double temperature;                                 // 측정된 온도
    std::chrono::steady_clock::time_point timestamp;    //생성된 정확한 시각
};

// 2. 동적 할당이 전혀 없는 고정 크기 링 버퍼
// 테스트를 눈으로 쉽게 확인하기 위해 크기를 5칸으로 먼저 만듦. (나중에 100으로 변경할 것)
constexpr size_t BUFFER_SIZE = 5;

class RingBuffer {
    private :
        std::array<SensorData, BUFFER_SIZE> buffer_; // 처음에 딱 고정된 메모리 공간
        size_t head_ = 0; // 데이터를 꺼낼 위치
        size_t tail_ = 0; // 데이터를 넣을 위치
        size_t count_ = 0; // 현재 들어있는 데이터 개수
    
    public :
        // 데이터를 넣는 함수 (성공하면 true, 꽉 차 있으면 false)
        bool push(const SensorData& data){
            if (count_ == BUFFER_SIZE) {
                return false; // 버퍼가 꽉 참
            }
            buffer_[tail_] = data;
            tail_ = (tail_ + 1) % BUFFER_SIZE; // 끝에 닿으면 다시 0번으로 회전
            count_++;
            return true;          
        }

        // 데이터를 꺼내는 함수 (성공하면 true, 비어 있으면 false)
        bool pop(SensorData& out_data) {
            if (count_ == 0) {
                return false;
            }
            out_data = buffer_[head_];
            head_ = (head_ + 1) % BUFFER_SIZE; // 끝에 닿으면 다시 0번으로 회전!
            count_--;
            return true; 
        }

        size_t size() const {return count_;}
};

// 3. 단일 스레드 입출력 테스트
int main() {
    SetConsoleOutputCP(65001);

    RingBuffer rb;

    std::cout << "=== [2단계] 링버퍼 회전 테스트 시작 (크기: " << BUFFER_SIZE << ") ===" << std::endl;

    // 1) 데이터 4개 연속 넣기
    for (int i = 1; i <= 4; ++i) {
        SensorData d{i, 25.0 + i, std::chrono::steady_clock::now()};
        if (rb.push(d)) {
            std::cout << "[Push 성공] ID: " << d.id << " (현재 버퍼 개수: " << rb.size() << ")\n";
        }
    }

    // 2) 데이터 2개 꺼내기 (앞쪽 0번, 1번 칸이 비워짐)
    SensorData out;
    for (int i = 0; i < 2; ++i) {
        if (rb.pop(out)) {
            std::cout << "  -> [Pop 성공] 꺼낸 ID: " << out.id << ", 온도: " << out.temperature << "도\n";
        }
    }

    // 3) 다시 데이터 3개 넣기 (배열 끝을 넘어 0번 칸으로 부드럽게 회전하는지 확인)
    for (int i = 5; i <= 7; ++i) {
        SensorData d{i, 30.0 + i, std::chrono::steady_clock::now()};
        if (rb.push(d)) {
            std::cout << "[회전 Push 성공] ID: " << d.id << " (현재 버퍼 개수: " << rb.size() << ")\n";
        } else {
            std::cout << "[Push 실패 - 버퍼 꽉 참] ID: " << d.id << "\n";
        }
    }

    return 0;
}