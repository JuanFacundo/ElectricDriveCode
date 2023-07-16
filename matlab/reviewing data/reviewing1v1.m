DATA = importdata("mock w motor piCAN_test1_0.txt");

N = DATA(:,1);
T81 = DATA(:,2);
T55 = DATA(:,3);

figure, hold on, plot(N,T81), plot(N,T55), hold off
