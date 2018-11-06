#!/usr/bin/env ruby

# Return map of { mach: …, re: …, ncrit: … }
def parse_mach_re_ncrit line
	s = line.split('=').map do |x|
		x.strip!
		x.split('  ')
	end.flatten
	s = s.map do |x|
		x.strip.gsub(' ', '')
	end
	s = s.reject &:empty?
	h = Hash[Hash[*s].map { |x, y| [x.downcase.to_sym, y.to_f] }]
	return h
end

def convert_input input
	reynolds = nil
	mode = :search_reynolds
	headers = []
	data = []
	for line in input.each_line
		line.strip!

		case mode
		when :search_reynolds
			if line.downcase.start_with? 'mach ='
				reynolds = parse_mach_re_ncrit(line)[:re]
				mode = :search_csv_header
			end
		when :search_csv_header
			line.downcase!
			if line.include? 'alpha' and line.include? 'top xtr'
				headers = line.split(',').map(&:to_sym)
				mode = :csv
			end
		when :csv
			unless line.strip.empty?
				data << line.gsub(' ', '').split(',').map(&:to_f)
			end
		end
	end

	field = {}
	0.upto(headers.size - 1) do |i|
		column = data.map { |x| x[i] }
		field[headers[i]] = column
	end

	return field.update({ reynolds: reynolds })
end

def print_polars polars, column, variable_identifier
	puts "#{variable_identifier} {"
	for polar in polars
		puts "\t{"
		puts "\t\t#{polar[:reynolds]},"
		puts "\t\t{"
		#puts "\t\t\t{ %8.3f_deg, %8.4f }," % [-180, 0.0] if column != :cl and column != :xcp
		polar[column].each_with_index do |value, i|
			puts "\t\t\t{ %8.3f_deg, %8.4f }," % [polar[:alpha][i], value]
		end
		#puts "\t\t\t{ %8.3f_deg, %8.4f }," % [+180, 0.0] if column != :cl
		puts "\t\t},"
		puts "\t},"
	end
	puts "};"
end

polars = []
namespace = ARGV[0]
for filename in ARGV[1..-1]
	polar = convert_input(File.read(filename))
	polar[:filename] = filename
	polars << polar
end
polars.sort! { |a, b| a[:reynolds] <=> b[:reynolds] }

puts "#include <xefis/utility/field.h>"
puts
puts "namespace #{namespace} {"
puts
print_polars polars, :cl,  "static xf::Field<double, si::Angle, double> const\nkLiftField"
puts
print_polars polars, :cd,  "static xf::Field<double, si::Angle, double> const\nkDragField"
puts
print_polars polars, :cm,  "static xf::Field<double, si::Angle, double> const\nkPitchingMomentField"
puts
print_polars polars, :xcp, "static xf::Field<double, si::Angle, double> const\nkCenterOfPressureOffsetField"
puts
puts "} // namespace #{namespace}"

