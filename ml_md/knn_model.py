from os import chdir
from pandas import read_csv
from sklearn.preprocessing import StandardScaler
from sklearn.model_selection import train_test_split
from imblearn.over_sampling import RandomOverSampler
from sklearn.neighbors import KNeighborsClassifier
from sklearn.metrics import classification_report

chdir(r'C:\Users\CaptainSwing817\projects\ddos')

df = read_csv('DDoS.csv')

df = df.dropna()

df['Label'] = (df['Label'] == 'DDoS').astype(int)

X = df.drop('Label', axis = 1)
Y = df['Label']

scaler = StandardScaler()
X = scaler.fit_transform(X)

X_train, X_test, Y_train, Y_test = train_test_split(X, Y, test_size = 0.2, random_state = 42)

ros = RandomOverSampler(random_state = 42)
X_train, Y_train = ros.fit_resample(X_train, Y_train)

knn = KNeighborsClassifier(n_neighbors = 5)
knn.fit(X_train, Y_train)

Y_pred = knn.predict(X_test)
print(classification_report(Y_test, Y_pred))
