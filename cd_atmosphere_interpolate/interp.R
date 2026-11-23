#!/usr/bin/env Rscript

# Small R script to interpolate the CD_ATMOSPHERE on to
# a regular grid and write it to a file.
# Niles Oien November 2026

args <- commandArgs(trailingOnly = TRUE)

# Validate argument count
if (length(args) < 2) {
  stop("Usage: interp.R <start depth Km> <end depth> <depth step> <output file>", call. = FALSE)
}

startDepth <- as.numeric(args[1])
endDepth <- as.numeric(args[2])
stepDepth <- as.numeric(args[3])
outFile <- args[4]

# Solar radius in cm
solar_radius <- 6.9599036e+10

# Read the multi-column table edited_cd_atmosphere that represents the solar atmosphere
df <- read.table("edited_cd_atmosphere", header = TRUE)

# Move from one minus r on R to depth in cm
df$one_minus_r_on_R <- df$one_minus_r_on_R * solar_radius
colnames(df)[colnames(df) == "one_minus_r_on_R"] <- "depth"

# Change depth units from cm to km
df$depth <- df$depth / 100000.0

# Define the original axis column and the target regular axis
x_col <- "depth"
x_orig <- df[[x_col]]

# Create a regular grid according to the command line args.
# Usually wind up running the program something like :
# ./interp.R 0.0 350000.0 250 interpolated_atmosphere_250
# going from depth 0.0Km to 350,000Km in steps of 250Km
# That's about the outer half of the radius of the sun.
x_reg <- seq(from=startDepth, to=endDepth, by=stepDepth)

# Identify all other columns to interpolate
y_cols <- setdiff(names(df), x_col)

# Interpolate each column onto the regular axis
interp_list <- lapply(df[y_cols], function(y) {
  approx(x = x_orig, y = y, xout = x_reg, method = "linear", rule = 1)$y
})

# Combine the new regular axis and interpolated columns into a data frame
df_interp <- data.frame(setNames(list(x_reg), x_col), interp_list)

# Write the result to a new file
# Increase width option so wide tables don't wrap onto new lines
options(width = 10000)

writeLines(
  capture.output(print(df_interp, row.names = FALSE, right = FALSE)),
  outFile
)

q(save='no')

