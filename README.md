# Unix-Project

client.c : client 최소 4개를 생성하여 1024*1024 크기의 data를 분산하고(분산된 데이터 파일로 저장) client끼리 통신하여 각자의 data를 모은 후 server에게 전달.
           server는 자신이 필요한 데이터를 모아 node파일로 저장.

server.c : client 최소 4개를 생성하여 1024*1024 크기의 data를 분산하고(파일로 저장) server에게 보내면 server측은 분산된 데이터를 하나로 모아, 자신이 필요한 data를 추출하여 node파일 생성 후 저장.

=> 두 가지의 통신기법을 이용하여 코딩한다, 또한 이 두 가지 통신기법의 실행시간을 비교한 후 분석한다.

client.c(pipe기법 이용) -> pipe는 통신에 크기가 작게 제한되어 있으므로, 여러개의 chunk로 데이터를 저장 후 통신 필요
server.c(mesQ-메시지큐 이용)
