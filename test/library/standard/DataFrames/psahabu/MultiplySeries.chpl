use DataFrames;

var I = new shared TypedIndex(["A", "B", "C", "D", "E"]);

var oneDigit = new owned TypedSeries([1, 2, 3, 4, 5], I);
var twoDigit = new owned TypedSeries([11, 22, 33, 44, 55], I);

writeln("factors:");
writeln(oneDigit);
writeln(twoDigit);

writeln("\nproduct:");
writeln(oneDigit * twoDigit);

var X = new owned TypedSeries([5, 1, 2], new shared TypedIndex(["A", "B", "C"]));
var Y = new owned TypedSeries([10, 20, 6, 7], new shared TypedIndex(["B", "C", "D", "E"]));

writeln("\nfactors:");
writeln(Y);
writeln(X);

writeln("\nproduct:");
writeln(X * Y);
