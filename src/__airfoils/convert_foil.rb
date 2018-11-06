#!/usr/bin/env ruby

def print_spline spline, variable_identifier
	puts "#{variable_identifier} {"
	for point in spline
		puts "\t{ %10.6f, %10.6f },\n" % [point[:x], point[:y]]
	end
	puts "};"
end

def convert_input input
	spline = []
	# Skip airfoil name:
	input.readline
	# Read points
	for line in input.each_line
		line.strip!
		line.squeeze!
		v = line.split(' ')
		spline << { x: v[0].to_f, y: v[1].to_f }
	end
	return spline
end

namespace = ARGV[0]
filename = ARGV[1]
spline = convert_input(File.open(filename))

puts "#include <xefis/support/simulation/airfoil_spline.h>"
puts
puts "namespace #{namespace} {"
puts
print_spline spline, "static xf::AirfoilSpline const\nkSpline"
puts
puts "} // namespace #{namespace}"

